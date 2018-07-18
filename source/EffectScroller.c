// Nordlicht demoparty
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "Tools.h"
#include <vshader_shbin.h>
#include "logo_scroller_bin.h"
#include "bg_scroller_bin.h"
#include "cubes_scroll_2_bin.h"
#include "cubes_scroll_bin.h"
#include "catface_bin.h"
#include "Catte.h"
#include "Perlin.h"

static DVLB_s* vshader_dvlb;
static shaderProgram_s program;

static int uLoc_projection, uLoc_modelview;
static C3D_Mtx projection;

static C3D_Tex screen_tex;
static C3D_Tex logo_tex;
static C3D_Tex cubes_tex;
static C3D_Tex cubes2_tex;
static C3D_Tex cat_tex;

static C3D_LightEnv lightEnv;
static C3D_Light light;
static C3D_LightLut lut_Phong;

int32_t vert_count;
static vertex* vbo_verts;
float* valueGrid;

#define MAX_VERTS 200000

#include "Font.h"
#include "MonoFont.h"

extern Font OL16Font; 

#define SCROLLERTEXT "TiTAN   k2   Eos   Alcatraz   Nuance   nordlicht   nordlicht   nordlicht"

static Pixel* scroll_pixels;
static Bitmap scroller;

const struct sync_track* sync_scroll_pos;
const struct sync_track* sync_bg_pos;

static const C3D_Material lightMaterial = {
    { 0.1f, 0.1f, 0.1f }, //ambient
    { 0.2f, 0.8f, 0.4f }, //diffuse
    { 0.8f, 0.8f, 0.9f }, //specular0
    { 0.0f, 0.0f, 0.0f }, //specular1
    { 0.0f, 0.1f, 0.0f }, //emission
};

// EXTREMELY simple scroller
void effectScrollerInit() {
    // Get rocket track
    sync_scroll_pos = sync_get_track(rocket, "scroller.pos");
    sync_bg_pos = sync_get_track(rocket, "scroller.bgpos");
    
    // Load default shader
    vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
    shaderProgramInit(&program);
    shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
    
    // Scroller texture
    scroll_pixels = (Pixel*)linearAlloc(512 * 512 * sizeof(Pixel));
    InitialiseBitmap(&scroller, 512, 512, BytesPerRowForWidth(512), scroll_pixels);
    
    // BG texture
    C3D_TexInit(&screen_tex, SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT, GPU_RGBA8);    
    C3D_TexUpload(&screen_tex, bg_scroller_bin);
    C3D_TexSetFilter(&screen_tex, GPU_NEAREST, GPU_NEAREST);
    
    // FG texture
    C3D_TexInit(&logo_tex, SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT, GPU_RGBA8);
    C3D_TexUpload(&logo_tex, logo_scroller_bin);
    C3D_TexSetFilter(&logo_tex, GPU_NEAREST, GPU_NEAREST);
    
    // Cube texture
    C3D_TexInit(&cubes_tex, 64, 64, GPU_RGBA8);
    C3D_TexUpload(&cubes_tex, cubes_scroll_bin);
    C3D_TexSetFilter(&cubes_tex, GPU_LINEAR, GPU_NEAREST);
    
    C3D_TexInit(&cubes2_tex, 64, 64, GPU_RGBA8);
    C3D_TexUpload(&cubes2_tex, cubes_scroll_2_bin);
    C3D_TexSetFilter(&cubes2_tex, GPU_LINEAR, GPU_NEAREST);
    
    // :3
    C3D_TexInit(&cat_tex, 64, 64, GPU_RGBA8);
    C3D_TexUpload(&cat_tex, catface_bin);
    C3D_TexSetFilter(&cat_tex, GPU_NEAREST, GPU_NEAREST);
    C3D_TexSetWrap(&cat_tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
    
    // Vertex Storage
    vbo_verts = (vertex*)linearAlloc(sizeof(vertex) * MAX_VERTS);
    C3D_BufInfo* bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, vbo_verts, sizeof(vertex), 3, 0x210);
    
    // Cat
    for(int f = 0; f < numFacesCatte; f++) {
        for(int v = 0; v < 3; v++) {
            // Set up vertex
            uint32_t vertexIndex = facesCatte[f].v[v];
            vbo_verts[f * 3 + v].position[0] = verticesCatte[vertexIndex].x;
            vbo_verts[f * 3 + v].position[1] = verticesCatte[vertexIndex].y;
            vbo_verts[f * 3 + v].position[2] = verticesCatte[vertexIndex].z;
            
            // Set normal to face normal
            vbo_verts[f * 3 + v].normal[0] = normalsCatte[facesCatte[f].v[3]].x;
            vbo_verts[f * 3 + v].normal[1] = normalsCatte[facesCatte[f].v[3]].y;
            vbo_verts[f * 3 + v].normal[2] = normalsCatte[facesCatte[f].v[3]].z;
            
            // Set texcoords
            vbo_verts[f * 3 + v].texcoord[0] = texcoordsCatte[vertexIndex].x;
            vbo_verts[f * 3 + v].texcoord[1] = texcoordsCatte[vertexIndex].y;
        }
    }
}


static void effectScrollerUpdate(float row) {
    // Set up vertices (externalize me if problem)
    vert_count = numFacesCatte * 3;
    for(int x = 0; x < 100; x++) {
        float offset = (x  - 50.0) * 0.037;
        offset = powf(offset, 7.0) * 0.1;
        
        for(int y = 0; y < 20; y++) {
            int pixel_val = (scroll_pixels[(20 - y) * 512 + x] >> 8) & 255;
            float noise_val = noise_at(x * 0.2, y * 0.2, row * 0.01);
            float noise_val_2 = noise_at(x * 0.4, y * 0.4, row * 0.02);
            float height = noise_val * 0.3 + pixel_val * 0.03 - 0.5;;
            float hh = 0.5;
            float vv = 0.0;
            if(height > 0.02) {
                hh = 0.0;
                vv = 0.5;
            } else {
                if(noise_val_2 > 0.0) {
                    hh = 0.0;                
                    vv = 0.0;
                }
                if(noise_val_2 > 0.2) {
                    hh = 0.5;
                    vv = 0.5;
                }
            }
            height += offset;
            
            vert_count += buildCuboid(vbo_verts + vert_count, vec3(x * 0.3, y * 0.3, height), vec3(0.3 / 2.0, 0.3 / 2.0, 0.5), hh, vv);
        }
    }
}

static void effectScrollerDraw(float iod, float row) {
    float bg_time = sync_get_val(sync_bg_pos, row);
    
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
    Mtx_PerspStereoTilt(&projection, 65.0f*M_PI/180.0f, 400.0f/240.0f, 0.2f, 30.0f, iod, 2.0f, false);
    
    // Calculate the modelview matrix
    C3D_Mtx modelview;
    Mtx_Identity(&modelview);
    Mtx_Translate(&modelview, -9.0, -8.5, -2.3, true);
    Mtx_RotateY(&modelview, 0.3, true);
    Mtx_RotateZ(&modelview, 0.4, true);
    Mtx_RotateX(&modelview, -1.0, true);
    
    // Update the uniforms
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelview,  &modelview);
    
    // Set up texops
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_RGB, GPU_FRAGMENT_PRIMARY_COLOR, GPU_FRAGMENT_SECONDARY_COLOR, 0);
    C3D_TexEnvOp(env, C3D_RGB, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_RGB, GPU_ADD);

    C3D_TexEnv* env2 = C3D_GetTexEnv(1);
    C3D_TexEnvSrc(env2, C3D_RGB, GPU_TEXTURE0, GPU_PREVIOUS, 0);
    C3D_TexEnvOp(env2, C3D_RGB, 0, 0, 0);
    C3D_TexEnvFunc(env2, C3D_RGB, GPU_MODULATE);
    
    C3D_TexEnv* env3 = C3D_GetTexEnv(2);
    C3D_TexEnvSrc(env3, C3D_RGB, GPU_TEXTURE0, GPU_PREVIOUS, 0);
    C3D_TexEnvOp(env3, C3D_RGB, 0, 0, 0);
    C3D_TexEnvFunc(env3, C3D_RGB, GPU_ADD);
    
    // Set up lighting equation
    static const C3D_Material lightMaterial = {
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
    
    C3D_FVec lightVec = { { 2.0, 2.0, -10.0, 0.0 } };

    C3D_LightInit(&light, &lightEnv);
    C3D_LightColor(&light, 1.0, 1.0, 1.0);
    C3D_LightPosition(&light, &lightVec);
    C3D_LightTwoSideDiffuse(&light, false);
    
    // Bind a texture
    C3D_TexBind(0, &cubes_tex);
    C3D_TexBind(1, &cubes2_tex);
    
    // Depth test on
    C3D_DepthTest(true, GPU_GREATER, GPU_WRITE_ALL);
        
    // Culling too
    C3D_CullFace(GPU_CULL_BACK_CCW);
    
    // Draw the VBO
    C3D_DrawArrays(GPU_TRIANGLES, numFacesCatte * 3, vert_count);
    
    // Texenv glowy
    env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_RGB, GPU_FRAGMENT_PRIMARY_COLOR, GPU_FRAGMENT_SECONDARY_COLOR, 0);
    C3D_TexEnvOp(env, C3D_RGB, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_RGB, GPU_ADD);

    env2 = C3D_GetTexEnv(1);
    C3D_TexEnvSrc(env2, C3D_RGB, GPU_TEXTURE0, GPU_PREVIOUS, 0);
    C3D_TexEnvOp(env2, C3D_RGB, 0, 0, 0);
    C3D_TexEnvFunc(env2, C3D_RGB, GPU_ADD);
    
    // Bind a texture
    C3D_TexBind(0, &cat_tex);
    
    // Calculate the modelview matrix (catte)
    srand(43);
    for(int i = 0; i < 6; i++) {
        float xoff = (rand() % 40) - 15.0;
        float zoff = ((rand() % 10) - 5.0) / 10.0;
        float toff = ((rand() % 300));
        float yoff = 0.0;
        if(bg_time * 0.01 < 30.0) {
            yoff = fmod(toff, 30) - 45.0 + bg_time * 0.01;
        }
        else {
            yoff = fmod(toff + bg_time * 0.01, 30) - 15.0;
        }
        Mtx_Identity(&modelview);
        Mtx_Translate(&modelview, xoff, 0, -15.0, true);
        Mtx_RotateX(&modelview, 0.3, true);    
        Mtx_RotateZ(&modelview, 0.4, true);
        Mtx_RotateY(&modelview, bg_time * 0.003, true);
        Mtx_Translate(&modelview, 0.0, yoff, 0.0, true);
        Mtx_Scale(&modelview, 2.0, 2.0, 2.0);
        
        // Update the uniforms
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelview,  &modelview);
        
        // Draw the VBO
        C3D_DrawArrays(GPU_TRIANGLES, 0, numFacesCatte * 3);
    }
    
    // Turn lighting off again
    C3D_LightEnvBind(0);
}

void effectScrollerRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float row) {
    effectScrollerUpdate(row);
    
    // Scroller draw
    float scroller_time = sync_get_val(sync_scroll_pos, row);
    float sshift = -scroller_time * 0.04;
    FillBitmap(&scroller, RGBAf(0.0, 0.0, 0.0, 0.0));
    DrawSimpleString(&scroller, &OL16Font, sshift, 6, RGBAf(0.3, 0.6 + 3 * 0.1, 1.0 - 3 * 0.3, 1.0), SCROLLERTEXT);

    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    
    // Left eye
    C3D_FrameDrawOn(targetLeft);
    
    // Background
    fullscreenQuad(screen_tex, -iod, 1.0 / 10.0);
    
    // Actual scene (empty in this, but you could!)
    effectScrollerDraw(-iod, row);
    
    // Overlay
    fullscreenQuad(logo_tex, 0.0, 0.0);
    
    fade();
    
    if(iod > 0.0) {
        // Right eye
        C3D_FrameDrawOn(targetRight);
    
        // Background
        fullscreenQuad(screen_tex, iod, 1.0 / 10.0);
        
        // Actual scene
        effectScrollerDraw(iod, row);
        
        // Overlay
        fullscreenQuad(logo_tex, 0.0, 0.0);
        
        fade();
    }
    
    C3D_FrameEnd(0);
}

void effectScrollerExit() {
    // Free vertices
    linearFree(vbo_verts);
    
    // Free the texture
    C3D_TexDelete(&screen_tex);
    C3D_TexDelete(&logo_tex);
    C3D_TexDelete(&cubes_tex);
    C3D_TexDelete(&cubes2_tex);
    
    // Free pixel data
    linearFree(scroll_pixels);
    
    // Free the shader program
    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);
}
