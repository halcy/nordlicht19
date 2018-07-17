// Pretty meta-balls.
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "Tools.h"
#include "vshader_shbin.h"
#include "cubes_bin.h"
#include "meta_fg_bin.h"
#include "Perlin.h"

#define METABALLS_MAX_VERTS 140000

static DVLB_s* vshader_dvlb;
static shaderProgram_s program;

static int uLoc_projection, uLoc_modelview;
static C3D_Mtx projection;

static Pixel* texPixels;
static Bitmap texture;
static C3D_Tex cubes_tex;

static C3D_LightEnv lightEnv;
static C3D_Light light;
static C3D_LightLut lut_Phong;
static C3D_LightLut lut_shittyFresnel;

static Pixel* screenPixels;
static Bitmap screen;
static C3D_Tex screen_tex;
static C3D_Tex logo_tex;

int32_t vertCount;
static vertex* vboVerts;

const struct sync_track* sync_obj_sel;
const struct sync_track* sync_rot_int;
const struct sync_track* sync_rot_ext;
const struct sync_track* sync_col;

#define GRID_X 11
#define GRID_Y 11
#define GRID_Z 11
#define GRID_STEP 0.05
#define GRID_EXTENT ((float)GRID_X * GRID_STEP)
#define GRID_OFFSET (GRID_EXTENT / 2.0)
#define GRID(x, y, z) ((x) + (y) * GRID_X + (z) * GRID_X * GRID_Y)

// A single metaball call.
// function:
// f(x,y,z) = 1 / ((x − x0)^2 + (y − y0)^2 + (z − z0)^2)
static inline float metaball(float x, float y, float z, float cx, float cy, float cz) {
    float dx = x - cx;
    float dy = y - cy;
    float dz = z - cz;
    float xs = dx * dx;
    float ys = dy * dy;
    float zs = dz * dz;
    return(1.0 / sqrt(xs + ys + zs));
}

// Metablobbies
void effectMetaobjectsInit() {
    // Rocket tracks
    sync_obj_sel = sync_get_track(rocket, "meta.object");
    sync_rot_int = sync_get_track(rocket, "meta.rot_int");
    sync_rot_ext = sync_get_track(rocket, "meta.rot_ext");
    sync_col = sync_get_track(rocket, "meta.col");
    
    // Load default shader
    vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
    shaderProgramInit(&program);
    shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);

    C3D_TexInit(&cubes_tex, 64, 64, GPU_RGBA8);
    C3D_TexUpload(&cubes_tex, cubes_bin);
    C3D_TexSetFilter(&cubes_tex, GPU_LINEAR, GPU_NEAREST);
    C3D_TexSetFilterMipmap(&cubes_tex, GPU_LINEAR);
    
    screenPixels = (Pixel*)linearAlloc(SCREEN_TEXTURE_WIDTH * SCREEN_TEXTURE_HEIGHT * sizeof(Pixel));
    InitialiseBitmap(&screen, SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT, BytesPerRowForWidth(SCREEN_TEXTURE_WIDTH), screenPixels);
    C3D_TexInit(&screen_tex, SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT, GPU_RGBA8);
    
    vboVerts = (vertex*)linearAlloc(sizeof(vertex) * METABALLS_MAX_VERTS);
    
    C3D_TexInit(&logo_tex, SCREEN_TEXTURE_HEIGHT, SCREEN_TEXTURE_WIDTH, GPU_RGBA8);
    C3D_TexUpload(&logo_tex, meta_fg_bin);
    C3D_TexSetFilter(&logo_tex, GPU_LINEAR, GPU_NEAREST);
}

static inline float field(float xx, float yy, float zz, float* xa, float* ya, float* za, int bc) {
    float val = 0.0;
    for(int i = 0; i < bc; i++) {
        val += metaball(xx, yy, zz, xa[i], ya[i], za[i]);
    }

    return(fmin(val / 10.0, 1.0));
}

static inline float field_torus(float xx, float yy, float zz) {
    float rad_large = 0.22;
    float rad_small = 0.1;    
    float aa = sqrt(xx*xx + zz*zz) - rad_large;
    float bb = sqrt(aa * aa + yy * yy);
    float dd = bb - rad_small;
    return dd < 0.0 ? fabs(dd) * 15.0 : 0.0;
}

static inline float field_box(float xx, float yy, float zz) {
    float rad = 0.3;
    float dd = fmax(fmax(fabs(xx), fabs(yy)), fabs(zz)) - rad;
    return dd < 0.0 ? fabs(dd) * 8.0 : 0.0;
}

void effectMetaobjectsPrepare(float row) {    
    // Movement.
    float movescale = 0.1;
    
    float xpos = GRID_OFFSET + cos(row * 0.3 * movescale) / 3.0;
    float ypos = GRID_OFFSET + sin(row * 0.2 * movescale) / 2.0;
    float zpos = GRID_OFFSET + sin(row * 0.36 * movescale) / 2.5;

    float xpos2 = GRID_OFFSET + sin(row * 0.22 * movescale) / 2.0;
    float ypos2 = GRID_OFFSET + cos(row * 0.07 * movescale) / 3.0;
    float zpos2 = GRID_OFFSET + cos(row * 0.19 * movescale) / 2.4;

    float xa[2] = {xpos, xpos2};
    float ya[2] = {ypos, ypos2};
    float za[2] = {zpos, zpos2};
    vertCount = 0;
    
    float rot_val = sync_get_val(sync_rot_int, row);
    mat3x3_t rot = mat3x3rotatez(rot_val * 0.05);
    rot = mat3x3mul(rot, mat3x3rotatex(rot_val * 0.025));
    
    int field_type = (int)sync_get_val(sync_obj_sel, row);
    int col_offset = (int)sync_get_val(sync_col, row);
    
    srand(666);
    if(field_type == 0) {
        for(int x = 0; x < GRID_X; x++) {
            for(int y = 0; y < GRID_Y; y++) {
                for(int z = 0; z < GRID_Z; z++) {
                    float xx = (float)x * GRID_STEP;
                    float yy = (float)y * GRID_STEP;
                    float zz = (float)z * GRID_STEP;

                    float rad = field(xx, yy, zz, xa, ya, za, 2);
                    int cubec = (rand() + col_offset) % 4;
                    float hh = cubec >= 2 ? 0.5 : 0.0;
                    float vv = cubec % 2 == 0 ? 0.0 : 0.5;
                    if(rad > 0.5) {
                        vertCount += buildCube(&vboVerts[vertCount], vec3(xx * 1.0, yy * 1.0, zz * 1.0), rad * 0.03, hh, vv);
                    }
                }
            }
        }
    }
    
    if(field_type == 1) {
        for(int x = 0; x < GRID_X; x++) {
            for(int y = 0; y < GRID_Y; y++) {
                for(int z = 0; z < GRID_Z; z++) {
                    float xx = (float)x * GRID_STEP;
                    float yy = (float)y * GRID_STEP;
                    float zz = (float)z * GRID_STEP;

                    vec3_t pv = vec3(xx - GRID_OFFSET, yy - GRID_OFFSET, zz - GRID_OFFSET);
                    pv = mat3x3transform(rot, pv);
                    float rad = field_box(pv.x, pv.y, pv.z);
                    
                    int cubec = (rand() + col_offset) % 4;
                    float hh = cubec >= 2 ? 0.5 : 0.0;
                    float vv = cubec % 2 == 0 ? 0.0 : 0.5;
                    if(rad > 0.5) {
                        vertCount += buildCube(&vboVerts[vertCount], vec3(xx * 1.0, yy * 1.0, zz * 1.0), rad * 0.03, hh, vv);
                    }
                }
            }
        }
    }
    
    if(field_type == 2) {
        for(int x = 0; x < GRID_X; x++) {
            for(int y = 0; y < GRID_Y; y++) {
                for(int z = 0; z < GRID_Z; z++) {
                    float xx = (float)x * GRID_STEP;
                    float yy = (float)y * GRID_STEP;
                    float zz = (float)z * GRID_STEP;

                    vec3_t pv = vec3(xx - GRID_OFFSET, yy - GRID_OFFSET, zz - GRID_OFFSET);
                    pv = mat3x3transform(rot, pv);
                    float rad = field_torus(pv.x, pv.y, pv.z);
                    
                    int cubec = (rand() + col_offset) % 4;
                    float hh = cubec >= 2 ? 0.5 : 0.0;
                    float vv = cubec % 2 == 0 ? 0.0 : 0.5;
                    if(rad > 0.5) {
                        vertCount += buildCube(&vboVerts[vertCount], vec3(xx * 1.0, yy * 1.0, zz * 1.0), rad * 0.03, hh, vv);
                    }
                }
            }
        }
    }
}

// Draw balls 
void effectMetabobjsDraw(float iod, float row) {    
    C3D_BindProgram(&program);

    // Get the location of the uniforms
    uLoc_projection = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
    uLoc_modelview = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");

    // Configure attributes for use with the vertex shader
    C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord
    AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 3); // v2=normal

    // Compute the projection matrix
    Mtx_PerspStereoTilt(&projection, 65.0f*M_PI/180.0f, 400.0f/240.0f, 0.01f, 1000.0f, iod, 2.0f, false);
    
    // Configure buffers
    C3D_BufInfo* bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, vboVerts, sizeof(vertex), 3, 0x210);
    
    C3D_TexBind(0, &cubes_tex);
    
    // Calculate the modelview matrix
    C3D_Mtx modelview;
    Mtx_Identity(&modelview);
    Mtx_Translate(&modelview, -0.0, -0.0, -0.9, true);
    float rot_val = sync_get_val(sync_rot_ext, row);
    Mtx_RotateY(&modelview, rot_val * 0.01, true);
    Mtx_RotateZ(&modelview, rot_val * 0.02, true);
    Mtx_Translate(&modelview, -GRID_OFFSET, -GRID_OFFSET, -GRID_OFFSET, true);
    
    // Update the uniforms
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelview,  &modelview);

    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_RGB, GPU_FRAGMENT_PRIMARY_COLOR, GPU_FRAGMENT_SECONDARY_COLOR, 0);
    C3D_TexEnvOp(env, C3D_RGB, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_RGB, GPU_ADD);

    C3D_TexEnv* env2 = C3D_GetTexEnv(1);
    C3D_TexEnvSrc(env2, C3D_RGB, GPU_TEXTURE0, GPU_PREVIOUS, 0);
    C3D_TexEnvOp(env2, C3D_RGB, 0, 0, 0);
    C3D_TexEnvFunc(env2, C3D_RGB, GPU_MODULATE);
    
    static const C3D_Material lightMaterial =
    {
        { 0.2f, 0.2f, 0.2f }, //ambient
        { 0.4f, 0.4f, 0.4f }, //diffuse
        { 0.8f, 0.8f, 0.8f }, //specular0
        { 0.0f, 0.0f, 0.0f }, //specular1
        { 0.0f, 0.0f, 0.0f }, //emission
    };

    C3D_LightEnvInit(&lightEnv);
    C3D_LightEnvBind(&lightEnv);
    C3D_LightEnvMaterial(&lightEnv, &lightMaterial);

    LightLut_Phong(&lut_Phong, 3.0);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_D0, GPU_LUTINPUT_LN, false, &lut_Phong);
    
    LightLut_FromFunc(&lut_shittyFresnel, badFresnel, 0.9, false);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_FR, GPU_LUTINPUT_NV, false, &lut_shittyFresnel);
    C3D_LightEnvFresnel(&lightEnv, GPU_PRI_SEC_ALPHA_FRESNEL);
    
    C3D_FVec lightVec = { { 2.0, 2.0, -10.0, 0.0 } };

    C3D_LightInit(&light, &lightEnv);
    C3D_LightColor(&light, 1.0, 1.0, 1.0);
    C3D_LightPosition(&light, &lightVec);
    C3D_LightTwoSideDiffuse(&light, true);
    
    // Depth test is back
    C3D_DepthTest(true, GPU_GREATER, GPU_WRITE_ALL);
    
    // To heck with culling
    C3D_CullFace(GPU_CULL_BACK_CCW);
    
    // Draw the VBO
    if(vertCount > 0) {
        C3D_DrawArrays(GPU_TRIANGLES, 0, vertCount);
    }
    
    C3D_LightEnvBind(0);
}

void effectMetaobjectsRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float row) { 
    // Render some 2D stuff
    // Blocky noise rotozoom
    FillBitmap(&screen, RGBAf(0.0, 0.0, 0.0, 1.0));        
    for(int x = 0; x < SCREEN_WIDTH; x += 10) {
        for(int y = 0; y < SCREEN_HEIGHT; y += 10) {
            float colVal = noise_at(x * 0.02, y * 0.02, row * 0.1);
            DrawFilledRectangle(&screen, x + 1, y + 1, 8, 8, RGBAf(colVal / 2.0, colVal, colVal, 1.0));
            //DrawFilledCircle(&screen, x + 5, y + 5, 4, RGBAf(colVal, colVal / 2.0, colVal, 1.0));
        }
    }
    
    GSPGPU_FlushDataCache(screenPixels, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(Pixel));
    GX_DisplayTransfer((u32*)screenPixels, GX_BUFFER_DIM(SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT), (u32*)screen_tex.data, GX_BUFFER_DIM(SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT), TEXTURE_TRANSFER_FLAGS);
    gspWaitForPPF();
    
    // Calculate ball vertices
    effectMetaobjectsPrepare(row);
    
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    
    // Left eye
    C3D_FrameDrawOn(targetLeft);
    
    // Fullscreen quad
    fullscreenQuad(screen_tex, -iod, 1.0 / 10.0);
    
    // Actual scene
    effectMetabobjsDraw(-iod, row);
    
    // Overlay
    fullscreenQuad(logo_tex, 0.0, 0.0);
    
    fade();
    
    if(iod > 0.0) {
        // Right eye
        C3D_FrameDrawOn(targetRight);
    
        // Fullscreen quad
        fullscreenQuad(screen_tex, iod, 1.0 / 10.0);
    
        // Actual scene
        effectMetabobjsDraw(iod, row);
        
        // Overlay
        fullscreenQuad(logo_tex, 0.0, 0.0);
        
        fade();
    }
    
    C3D_FrameEnd(0);
}

void effectMetaobjectsExit() {
    // Free the texture
    C3D_TexDelete(&cubes_tex);
    C3D_TexDelete(&screen_tex);
    C3D_TexDelete(&logo_tex);

    // Free the VBOs
    linearFree(vboVerts);
    
    // Free the shader program
    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);
    
    // Free the bitmaps
    linearFree(screenPixels);
    linearFree(texPixels);
}
