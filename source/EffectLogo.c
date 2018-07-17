// Nordlicht demoparty
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "Tools.h"
#include <vshader_shbin.h>
#include "logo_fg_bin.h"
#include "tunnel_glow_bin.h"
#include "Perlin.h"

#include "Logo.h"

static DVLB_s* vshader_dvlb;
static shaderProgram_s program;

static int uLocProjection;
static int uLocModelview;
static C3D_Mtx projection;

static Pixel* screenPixels;
static Bitmap screen;
static C3D_Tex screen_tex;
static C3D_Tex logo_tex;
static C3D_Tex projTex;

static C3D_LightEnv lightEnv;
static C3D_Light light;
static C3D_LightLut lutShittyFresnel;
static C3D_LightLut lutPhong;

static C3D_FogLut fog_Lut;

static Pixel* projPixels;
static Bitmap projBitmap;
static C3D_Tex projTex;

const struct sync_track* sync_bg_pos;
const struct sync_track* sync_stripe_pos;
const struct sync_track* sync_stripe_alpha;
const struct sync_track* sync_stripe_active;
const struct sync_track* sync_blend;
const struct sync_track* sync_displace;
const struct sync_track* sync_displace_pow;

static const C3D_Material lightMaterial = {
    { 0.1f, 0.1f, 0.1f }, //ambient
    { 0.2f, 0.8f, 0.4f }, //diffuse
    { 0.8f, 0.8f, 0.9f }, //specular0
    { 0.0f, 0.0f, 0.0f }, //specular1
    { 0.0f, 0.1f, 0.0f }, //emission
};

static vertex* logoVBO;
int32_t vertCount;
#define MAX_VERTS 40000

void effectLogoInit() {
    // Get rocket track
    sync_bg_pos = sync_get_track(rocket, "logo.bgpos");
    sync_stripe_pos = sync_get_track(rocket, "logo.stripepos");
    sync_stripe_alpha = sync_get_track(rocket, "logo.stripealpha");
    sync_stripe_active = sync_get_track(rocket, "logo.stripes");
    sync_blend = sync_get_track(rocket, "logo.blend");
    sync_displace = sync_get_track(rocket, "logo.displace");
    sync_displace_pow = sync_get_track(rocket, "logo.displacepow");
    
    
    // Load default shader
    vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
    shaderProgramInit(&program);
    shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);

    // BG texture
    C3D_TexInit(&screen_tex, SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT, GPU_RGBA8);    
    screenPixels = (Pixel*)linearAlloc(SCREEN_TEXTURE_WIDTH * SCREEN_TEXTURE_HEIGHT * sizeof(Pixel));
    InitialiseBitmap(&screen, SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT, BytesPerRowForWidth(SCREEN_TEXTURE_WIDTH), screenPixels);
    
    // FG texture
    C3D_TexInit(&logo_tex, SCREEN_TEXTURE_HEIGHT, SCREEN_TEXTURE_WIDTH, GPU_RGBA8);
    C3D_TexUpload(&logo_tex, logo_fg_bin);
    C3D_TexSetFilter(&logo_tex, GPU_LINEAR, GPU_NEAREST);
    
    // Tunnel texture
    C3D_TexInit(&projTex, 128, 128, GPU_RGBA8);
    projPixels = (Pixel*)linearAlloc(128 * 128 * sizeof(Pixel));
    InitialiseBitmap(&projBitmap, 128, 128, BytesPerRowForWidth(128), projPixels);
    
    // Create the VBO
    logoVBO = (vertex*)linearAlloc(sizeof(vertex) * MAX_VERTS);
    
    C3D_BufInfo* bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, (void*)logoVBO, sizeof(vertex), 3, 0x210);
    
//     C3D_LightEnvInit(&lightEnv);
//     C3D_LightEnvBind(&lightEnv);
//     C3D_LightEnvMaterial(&lightEnv, &lightMaterial);
// 
//     LightLut_Phong(&lutPhong, 20.0f);
//     C3D_LightEnvLut(&lightEnv, GPU_LUT_D0, GPU_LUTINPUT_NH, false, &lutPhong);
// 
//     C3D_LightInit(&light, &lightEnv);
}


void setNewVerts(
    vertex* vbo, 
    const int numVertices, 
    const int numFaces, 
    const init_vertex_t* vertices, 
    const init_vertex_t* normals, 
    const index_triangle_t* faces, 
    float displace,
    float displace_pow
) {
    int32_t* normalIndices = (int32_t*)malloc(sizeof(int32_t) * numVertices);
    for(int i = 0; i < numVertices; i++) {
        normalIndices[i] = -1;
    }
    for(int f = 0; f < numFaces; f++) {
        for(int v = 0; v < 3; v++) {
            // Set up vertex
            uint32_t vertexIndex = faces[f].v[v];
            vbo[f * 3 + v].position[0] = vertices[vertexIndex].x;
            vbo[f * 3 + v].position[1] = vertices[vertexIndex].y;
            vbo[f * 3 + v].position[2] = vertices[vertexIndex].z;
            
            // Set normal to face normal
            vbo[f * 3 + v].normal[0] = normals[faces[f].v[3]].x;
            vbo[f * 3 + v].normal[1] = normals[faces[f].v[3]].y;
            vbo[f * 3 + v].normal[2] = normals[faces[f].v[3]].z;
            
            // Displace
            float shiftVal = noise_at(
                (2.0 + vertices[vertexIndex].x) * 4.0, 
                (1.0 + vertices[vertexIndex].y) * 4.0, 
                (2.0 + vertices[vertexIndex].z) * 4.0 + displace * 0.0005
            ) / 6.0;
            shiftVal = fmax(shiftVal, 0.0) * sin(displace * 0.0005) * displace_pow;
            
            int32_t normalIndex = faces[f].v[3];
            if(normalIndices[vertexIndex] == -1) {
                normalIndices[vertexIndex] = normalIndex;
            }
            else {
                normalIndex = normalIndices[vertexIndex];
            }
            
            vbo[f * 3 + v].position[0] += shiftVal * normals[normalIndex].x;
            vbo[f * 3 + v].position[1] += shiftVal * normals[normalIndex].y;
            vbo[f * 3 + v].position[2] += shiftVal * normals[normalIndex].z;
            
            // Set texcoords
            vbo[f * 3 + v].texcoord[0] = vbo[f * 3 + v].position[0] * 5.0 + 0.5;
            vbo[f * 3 + v].texcoord[1] = vbo[f * 3 + v].position[1] * 5.0 + 0.5;
        }
    }
    free(normalIndices);
    
    // Recompute normals
    for(int f = 0; f < numFaces; f++) {
        vec3_t a = vec3(vbo[f * 3 + 0].position[0], vbo[f * 3 + 0].position[1], vbo[f * 3 + 0].position[2]);
        vec3_t b = vec3(vbo[f * 3 + 1].position[0], vbo[f * 3 + 1].position[1], vbo[f * 3 + 1].position[2]);
        vec3_t c = vec3(vbo[f * 3 + 2].position[0], vbo[f * 3 + 2].position[1], vbo[f * 3 + 2].position[2]);
        vec3_t n = vec3norm(vec3cross(vec3norm(vec3sub(b, a)), vec3norm(vec3sub(c, a))));
        for(int v = 0; v < 3; v++) {
            vbo[f * 3 + v].normal[0] = n.x;
            vbo[f * 3 + v].normal[1] = n.y;
            vbo[f * 3 + v].normal[2] = n.z;
        }
    }
}

void effectLogoDraw(float iod, float row) {
    C3D_BindProgram(&program);
    
    // Set GPU state
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
    
    // Get the location of the uniforms
    uLocProjection = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
    uLocModelview = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");

    // Configure attributes for use with the vertex shader
    C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0 = position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1 = texcoord
    AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 3); // v2 = normal

    // Compute the projection matrix
    Mtx_PerspStereoTilt(&projection, 65.0f*M_PI/180.0f, 400.0f/240.0f, 0.2f, 500.0f, iod, 2.0f, false);
    
    // Bind a texture
    C3D_TexSetFilter(&projTex, GPU_LINEAR, GPU_NEAREST);
    C3D_TexSetWrap(&projTex, GPU_REPEAT, GPU_REPEAT);
    C3D_TexBind(0, &projTex);
    
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_RGB, GPU_FRAGMENT_PRIMARY_COLOR, GPU_FRAGMENT_SECONDARY_COLOR, 0);
    C3D_TexEnvOp(env, C3D_RGB, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_RGB, GPU_ADD);

    C3D_TexEnv* env2 = C3D_GetTexEnv(1);
    C3D_TexEnvSrc(env2, C3D_RGB, GPU_TEXTURE0, GPU_PREVIOUS, 0);
    C3D_TexEnvOp(env2, C3D_RGB, 0, 0, 0);
    C3D_TexEnvFunc(env2, C3D_RGB, GPU_MODULATE);
    
    C3D_TexEnv* env3 = C3D_GetTexEnv(2);
    C3D_TexEnvSrc(env3, C3D_RGB, GPU_FRAGMENT_SECONDARY_COLOR, GPU_PREVIOUS, 0);
    C3D_TexEnvOp(env3, C3D_RGB, GPU_TEVOP_RGB_SRC_ALPHA , 0, 0);
    C3D_TexEnvFunc(env3, C3D_RGB, GPU_ADD);
    
    static const C3D_Material lightMaterial =
    {
        { 0.2f, 0.0f, 0.0f }, //ambient
        { 0.4f, 0.4f, 0.4f }, //diffuse
        { 0.8f, 0.8f, 0.8f }, //specular0
        { 0.0f, 0.0f, 0.0f }, //specular1
        { 0.0f, 0.0f, 0.0f }, //emission
    };

    C3D_LightEnvInit(&lightEnv);
    C3D_LightEnvBind(&lightEnv);
    C3D_LightEnvMaterial(&lightEnv, &lightMaterial);

    LightLut_Phong(&lutPhong, 3.0);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_D0, GPU_LUTINPUT_LN, false, &lutPhong);
    
    LightLut_FromFunc(&lutShittyFresnel, badFresnel, 1.9, false);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_FR, GPU_LUTINPUT_NV, false, &lutShittyFresnel);
    C3D_LightEnvFresnel(&lightEnv, GPU_PRI_SEC_ALPHA_FRESNEL);
    
    C3D_FVec lightVec = { { 10.0, 0.0, 0.0, 0.0 } };
    C3D_LightInit(&light, &lightEnv);
    C3D_LightColor(&light, 1.0, 1.0, 1.0);
    C3D_LightPosition(&light, &lightVec);
    
    // Compute new modelview
    C3D_Mtx modelview;
    Mtx_Identity(&modelview);
    Mtx_Translate(&modelview, 0.1, 0.2, -2.1, true);
    Mtx_RotateY(&modelview, -1.1, true);
    Mtx_Scale(&modelview, 10.0, 10.0, 10.0);
    
    // Send matrices
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocProjection, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocModelview,  &modelview);

    C3D_CullFace(GPU_CULL_NONE);
    
    // Stream some new verts
    float displace = sync_get_val(sync_displace, row);
    float displace_pow = sync_get_val(sync_displace_pow, row);
    setNewVerts(logoVBO, numVerticesLogo, numFacesLogo, verticesLogo, normalsLogo, facesLogo, displace, displace_pow);
    setNewVerts(&logoVBO[numFacesLogo * 3], numVerticesLogoNordlicht, numFacesLogoNordlicht, verticesLogoNordlicht, normalsLogoNordlicht, facesLogoNordlicht, displace, displace_pow);
    
    // Additive blended draw
    float logo_blend = sync_get_val(sync_blend, row);
    int draw_start = 0 * (1.0 - logo_blend) + (numFacesLogo * 3) * logo_blend;
    int draw_end = + (numFacesLogo * 3) * (1.0 - logo_blend) + (numFacesLogoNordlicht * 3 + numFacesLogo * 3)  * logo_blend;
    draw_start = (draw_start / 3) * 3;
    draw_end = (draw_end / 3) * 3;
    
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
    C3D_DrawArrays(GPU_TRIANGLES, draw_start, draw_end);
}

void effectLogoRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float row) {
    float bg_time = sync_get_val(sync_bg_pos, row);
    float xshift = cos(bg_time * 0.0003) * 0.1;
    float yshift = sin(bg_time * 0.0001) * 0.1;
    
    
    // Update projective "cutting" texture
    FillBitmap(&projBitmap, RGBA(163, 73, 164, 255));
    if(sync_get_val(sync_stripe_active, row) > 0.0) {
        int32_t offset = (int32_t)(sync_get_val(sync_stripe_pos, row)) % 14;
        int32_t stripe_alpha = sync_get_val(sync_stripe_alpha, row);
        for(int y = 0; y < 128; y += 14) {
            DrawFilledRectangle(&projBitmap, 0, y + offset, 128, 7, RGBA(0, 0, 0, stripe_alpha));
        }
    }
    
    GSPGPU_FlushDataCache(projPixels, 128 * 128 * sizeof(Pixel));
    GX_DisplayTransfer((u32*)projPixels, GX_BUFFER_DIM(128, 128), (u32*)projTex.data, GX_BUFFER_DIM(128, 128), TEXTURE_TRANSFER_FLAGS);
    gspWaitForPPF();
    
    // Render some 2D stuff
    FillBitmap(&screen, RGBAf(0.1 * 0.5, 0.15 * 0.5, 0.15 * 0.5, 1.0));
    for(int x = 0; x < SCREEN_WIDTH; x += 10) {
        for(int y = 0; y < SCREEN_HEIGHT; y += 10) {
            float posX = ((float)(x - SCREEN_WIDTH / 2) / (float)SCREEN_WIDTH) + xshift * 0.1;
            float posY = ((float)(y - SCREEN_HEIGHT / 2) / (float)SCREEN_WIDTH) - yshift * 0.1;
            
            float lines = fmod(posX + posY * 0.3 + bg_time * 0.0001 + 10.0, 0.3) > 0.15 ? 0.15 : 0.05;
            float lines2 = fmod(posX + posY * 0.2 + bg_time * 0.0002 + 10.0, 0.3) > 0.15 ? 0.55 : 0.05;
            
            Pixel colorPrimary = RGBAf(lines2, lines2 * 0.9, lines2, 1.0);
            Pixel colorSecondary = RGBAf(lines, lines, lines, 1.0);
            
            DrawFilledRectangle(&screen, x, y, 10, 10, colorSecondary);
            DrawFilledCircle(&screen, x + 5, y + 5, 3, colorPrimary);
        }
    }
    
    GSPGPU_FlushDataCache(screenPixels, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(Pixel));
    GX_DisplayTransfer((u32*)screenPixels, GX_BUFFER_DIM(SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT), (u32*)screen_tex.data, GX_BUFFER_DIM(SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT), TEXTURE_TRANSFER_FLAGS);
    gspWaitForPPF();
    
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    
    // Left eye
    C3D_FrameDrawOn(targetLeft);
//     C3D_RenderTargetClear(targetLeft, C3D_CLEAR_ALL, 0x68B0D8FF, 0);
    
    // Background
    fullscreenQuad(screen_tex, -iod, 1.0 / 10.0);
    
    // Actual scene (empty in this, but you could!)
    effectLogoDraw(-iod, row);
    
    // Overlay
    fullscreenQuad(logo_tex, 0.0, 0.0);
  
    fade();
    
    if(iod > 0.0) {
        // Right eye
        C3D_FrameDrawOn(targetRight);
//         C3D_RenderTargetClear(targetRight, C3D_CLEAR_ALL, 0x68B0D8FF, 0);
        
        // Background
        fullscreenQuad(screen_tex, iod, 1.0 / 10.0);
        
        // Actual scene
        effectLogoDraw(iod, row);
    
        // Overlay
        fullscreenQuad(logo_tex, 0.0, 0.0);
        
        fade();
    }
    
    C3D_FrameEnd(0);
}

void effectLogoExit() {
    // Free the texture
    C3D_TexDelete(&screen_tex);
    C3D_TexDelete(&logo_tex);
    C3D_TexDelete(&projTex);
    
    // Free vertices
    linearFree(logoVBO);
    
    // Free pixel data
    linearFree(screenPixels);
    linearFree(projPixels);
    
    // Free the shader program
    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);
}
