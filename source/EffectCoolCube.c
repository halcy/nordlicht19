// Nordlicht demoparty
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "Tools.h"

#include "ModelCoolCube.h"

#include "Font.h"
#include "MonoFont.h"

extern Font OL16; 

#define SCROLLERTEXT "              k2          TiTAN          SunSpire          Nuance          cncd          logicoma          mercury          spacepigs          jvb          dotUser         fincs           fuckings 2 lamers"
// k2
// TiTAN
// SunSpire
// Nuance
// cncd
// logicoma
// mercury
// spacepigs
// jvb
// dotUser
// Shader / textures
static C3D_AttrInfo* attrInfo;

static int uLocProjection;
static int uLocModelview;
static int uLocProjectionSkybox;
static int uLocModelviewSkybox;

static C3D_Tex skybox_tex;
static C3D_TexCube skybox_cube;
int skyboxVertCount;

int cometsStart;
int cometsVertCount;
#define TAIL_LEN 100

static Pixel* cubeColPixels;
static Bitmap cubeCol;
static C3D_Tex cubeColTex;

static Pixel* cubeNormPixels;
static Bitmap cubeNorm;
static C3D_Tex cubeNormTex;

static Pixel* cubeColAddPixels;
static Bitmap cubeColAdd;
static C3D_Tex cubeColAddTex;

// Lighting
static C3D_LightEnv lightEnv;
static C3D_Light light[3];
static C3D_LightLut lutShittyFresnel;
static C3D_LightLut lutPhong;

int initial = 1;

static const C3D_Material lightMaterial = {
    { 0.1f, 0.0f, 0.0f }, //ambient
    { 1.0f, 1.0f, 1.0f }, //diffuse
    { 0.4f, 0.4f, 0.4f }, //specular0
    { 0.0f, 0.0f, 0.0f }, //specular1
    { 0.0f, 0.0f, 0.0f }, //emission
};

// VBOs
#define VBO_SIZE 35000
static vertex2* vbo;
static C3D_BufInfo* bufInfo;

// Sync
const struct sync_track* sync_zoom;
const struct sync_track* sync_rotate;
const struct sync_track* sync_comets;
const struct sync_track* sync_scroll;

void effectCoolCubeInit() {
//     printf("INIT BEGINS.\n");
    // initialize everything here
    sync_zoom = sync_get_track(rocket, "cube.zoom");
    sync_rotate = sync_get_track(rocket, "cube.rotate");
    sync_comets = sync_get_track(rocket, "cube.comets");
    sync_scroll = sync_get_track(rocket, "cube.scroller");
    
    // Set up shaders and get uniform locations
    C3D_BindProgram(&shaderProgramNormalMapping);
    uLocProjection = shaderInstanceGetUniformLocation(shaderProgramNormalMapping.vertexShader, "projection");
    uLocModelview = shaderInstanceGetUniformLocation(shaderProgramNormalMapping.vertexShader, "modelView");
    
    C3D_BindProgram(&shaderProgramSkybox);
    uLocProjectionSkybox = shaderInstanceGetUniformLocation(shaderProgramSkybox.vertexShader, "projection");
    uLocModelviewSkybox = shaderInstanceGetUniformLocation(shaderProgramSkybox.vertexShader, "modelView");
    
    // Allocate VBOs
    printf("Lin Alloc.\n");
    waitForA();    
    vbo = (vertex2*)linearAlloc(sizeof(vertex2) * VBO_SIZE);
    
    bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, (void*)vbo, sizeof(vertex2), 4, 0x3210);
    
    // Load statics
    loadObject2(numFacesCoolCube, facesCoolCube, verticesCoolCube, normalsCoolCube, texcoordsCoolCube, &vbo[0]);
    for(int i = 0; i < numFacesCoolCube * 3; i++) {
        vbo[i].texcoord[1] *= 2.0;
    }
    skyboxVertCount = buildCube2(&vbo[(numFacesCoolCube) * 3], vec3(0, 0, 0), 29500.0, 0.0, 0.0);
    cometsStart = (numFacesCoolCube) * 3 + skyboxVertCount;
//     printf("Crash WAY early.\n");
    // Load texture for the skybox
    loadTex3DS(&skybox_tex, &skybox_cube, "romfs:/stars_red.bin");
    C3D_TexSetFilter(&skybox_tex, GPU_LINEAR, GPU_LINEAR);
    C3D_TexSetWrap(&skybox_tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
    
    printf("Tex A.\n");
    waitForA(); 
    
    // Cool Cube textures
    cubeColPixels = (Pixel*)linearAlloc(512 * 256 * sizeof(Pixel));
    InitialiseBitmap(&cubeCol, 512, 256, BytesPerRowForWidth(512), cubeColPixels);
    C3D_TexInitVRAM(&cubeColTex, 512, 256, GPU_RGBA8);
    C3D_TexSetFilter(&cubeColTex, GPU_LINEAR, GPU_NEAREST);
    C3D_TexSetWrap(&cubeColTex, GPU_REPEAT, GPU_REPEAT);  
    
    printf("Tex B.\n");
    waitForA(); 
    
    cubeNormPixels = (Pixel*)linearAlloc(512 * 256 * sizeof(Pixel));
    InitialiseBitmap(&cubeNorm, 512, 256, BytesPerRowForWidth(512), cubeNormPixels);
    C3D_TexInitVRAM(&cubeNormTex, 512, 256, GPU_RGBA8);
    C3D_TexSetFilter(&cubeNormTex, GPU_LINEAR, GPU_NEAREST);
    C3D_TexSetWrap(&cubeNormTex, GPU_REPEAT, GPU_REPEAT);  
    
    printf("Tex C.\n");
    waitForA(); 
    
    cubeColAddPixels = (Pixel*)linearAlloc(512 * 256 * sizeof(Pixel));
    InitialiseBitmap(&cubeColAdd, 512, 256, BytesPerRowForWidth(512), cubeColAddPixels);
    C3D_TexInitVRAM(&cubeColAddTex, 512, 256, GPU_RGBA8);
    C3D_TexSetFilter(&cubeColAddTex, GPU_LINEAR, GPU_NEAREST);
    C3D_TexSetWrap(&cubeColAddTex, GPU_REPEAT, GPU_REPEAT);
    
//     printf("Music load ----- %d %d %d\n", linearSpaceFree(), vramSpaceFree(), mappableSpaceFree());
    
    initial = 1;    
}

vec3_t sph_sinusoid(float a, float k, float n, float t) {
    float cosnt = cos(n * t);
    float norm =  sqrt(1.0 + k * 2 + cosnt * cosnt);
    
    float x = a * cos(t) / norm;
    float y = a * sin(t) / norm;
    float z = a * k * cosnt / norm;
    
    return vec3(x, y, z);
}

void effectCoolCubeUpdate(float row) {
    printf("FREE IN UPD----- %d %d %d\n", linearSpaceFree(), vramSpaceFree(), mappableSpaceFree());
    
    float sync_comets_val = sync_get_val(sync_comets, row);
    cometsVertCount = 0;
    for(int i = 0; i < 3; i++) {
        for(int j = 0; j < TAIL_LEN; j++) {
          float j_scale = 0.01;
          float t = (i / 3.0) * 2.0 * 3.14152 + sync_comets_val + j_scale * j;
          
          float jr = (float)(TAIL_LEN - j) / TAIL_LEN;
          double size = -2.775558e-16 + 10.53333*jr - 35.06667*jr*jr + 39.46667*jr*jr*jr- 14.93333*jr*jr*jr*jr;
          
          t = ((int)(t / j_scale)) * j_scale;
          float rad = 11.0 + 0.5 * sin(t + 5.0 * i);
          
          /*float x = sin(t) + 2.0 * sin(2.0 * t); // TODO
          float y = cos(t) - 2.0 * cos(2.0 * t);
          float z = -sin(3.0 * t);*/
          vec3_t pos = sph_sinusoid(rad, 2 + i, 5, t);
          
          if(i == 0) {
              float temp = pos.x;
              pos.x = pos.z;
              pos.z = temp;
          }
          
          if(i == 1) {
              float temp = pos.z;
              pos.z = pos.y;
              pos.y = temp;
          }
          
//           float st = t / j_scale;
//           srand(st - fmod(st, j_scale));
          srand(t / j_scale);
          
          pos.x += (((rand() % 128) / 128.0) - 0.5) * 0.3;
          pos.y += (((rand() % 128) / 128.0) - 0.5) * 0.3;
          pos.z += (((rand() % 128) / 128.0) - 0.5) * 0.3;
          
          cometsVertCount += buildCube2(&vbo[cometsStart + cometsVertCount], pos, 0.4 * size , 0.0, 0.0);
        }
    }
//     printf("COMETS DONE %d + %d = %d.\n", cometsStart, cometsVertCount, cometsStart + cometsVertCount);
//     waitForA();
//     
    float sync_scroll_val = sync_get_val(sync_scroll, row);
    
    // Render some 2D stuff
    if(initial == 1) {
        FillBitmap(&cubeCol, RGBAf(0.8, 0.8, 0.8, 1.0));    
    }
    for(int i = 0; i < 19; i++) {
        DrawHorizontalLine(&cubeCol, 0, i, 512, RGBAf(0.1, 0.1, 0.1, 0.3));
    }
    DrawSimpleString(&cubeCol, &OL16, sync_scroll_val, 1, RGBAf(1.0, 0.1, 0.2, 1.0), SCROLLERTEXT);
    
    if(initial == 1) {
        GSPGPU_FlushDataCache(cubeColPixels, 512 * 256 * sizeof(Pixel));
        GX_DisplayTransfer((u32*)cubeColPixels, GX_BUFFER_DIM(512, 256), (u32*)cubeColTex.data, GX_BUFFER_DIM(512, 256), TEXTURE_TRANSFER_FLAGS);
    }
    else {
        GSPGPU_FlushDataCache(cubeColPixels, 512 * 64 * sizeof(Pixel));
        GX_DisplayTransfer((u32*)cubeColPixels, GX_BUFFER_DIM(512, 64), (u32*)cubeColTex.data, GX_BUFFER_DIM(512, 64), TEXTURE_TRANSFER_FLAGS);
    }
    gspWaitForPPF();
    
    if(initial == 1) {
        FillBitmap(&cubeColAdd, RGBAf(0.0, 0.0, 0.0, 0.0));    
    }
    for(int i = 0; i < 19; i++) {
        DrawHorizontalLine(&cubeColAdd, 0, i, 512, RGBAf(0.0, 0.0, 0.0, 0.0));
    }
    DrawSimpleString(&cubeColAdd, &OL16, sync_scroll_val, 1, RGBAf(1.0, 0.1, 0.2, 1.0), SCROLLERTEXT);
    
    if(initial == 1) {
        GSPGPU_FlushDataCache(cubeColAddPixels, 512 * 256 * sizeof(Pixel));
        GX_DisplayTransfer((u32*)cubeColAddPixels, GX_BUFFER_DIM(512, 256), (u32*)cubeColAddTex.data, GX_BUFFER_DIM(512, 256), TEXTURE_TRANSFER_FLAGS);
    }
    else {
        GSPGPU_FlushDataCache(cubeColAddPixels, 512 * 64 * sizeof(Pixel));
        GX_DisplayTransfer((u32*)cubeColAddPixels, GX_BUFFER_DIM(512, 64), (u32*)cubeColAddTex.data, GX_BUFFER_DIM(512, 64), TEXTURE_TRANSFER_FLAGS);
    }
    gspWaitForPPF();
    
    if(initial == 1) {
        FillBitmap(&cubeNorm, RGBAf(0.5, 0.5, 1.0, 1.0));
    }
    DrawSimpleString(&cubeNorm, &OL16, sync_scroll_val + 1, 2, RGBAf(0.75, 0.45, 0.95, 1.0), SCROLLERTEXT);
    DrawSimpleString(&cubeNorm, &OL16, sync_scroll_val - 1, 0, RGBAf(0.45, 0.75, 0.95, 1.0), SCROLLERTEXT);
    DrawSimpleString(&cubeNorm, &OL16, sync_scroll_val, 1, RGBAf(0.5, 0.5, 1.0, 1.0), SCROLLERTEXT);
    
    // TODO put an actual normal map here
    if(initial == 1) {
        for(int i = 32; i < 200; i += 64) {
          for(int j = 0; j < 8; j++) {
              DrawHorizontalLine(&cubeNorm, 0, i+j, 512, RGBAf(0.1, 0.1, 0.2, 1.0));
          }
          for(int j = 8; j < 16; j++) {
              DrawHorizontalLine(&cubeNorm, 0, i+j, 512, RGBAf(0.5, 0.5, 1.0, 1.0));
          }
        }
    }
    
    if(initial == 1) {
        GSPGPU_FlushDataCache(cubeNormPixels, 512 * 256 * sizeof(Pixel));
        GX_DisplayTransfer((u32*)cubeNormPixels, GX_BUFFER_DIM(512, 256), (u32*)cubeNormTex.data, GX_BUFFER_DIM(512, 256), TEXTURE_TRANSFER_FLAGS);
    }
    else {
        GSPGPU_FlushDataCache(cubeNormPixels, 512 * 64 * sizeof(Pixel));
        GX_DisplayTransfer((u32*)cubeNormPixels, GX_BUFFER_DIM(512, 64), (u32*)cubeNormTex.data, GX_BUFFER_DIM(512, 64), TEXTURE_TRANSFER_FLAGS);
    }
    
    gspWaitForPPF();
    initial = 0;
//     printf("exit update.\n");
//     waitForA();
}

void effectCoolCubeDraw(float iod, float row) {
//     printf("DRAW.\n");
    resetShadeEnv();
    float sync_zoom_val = sync_get_val(sync_zoom, row);
    float sync_rotate_val = sync_get_val(sync_rotate, row);
    float sync_comets_val = sync_get_val(sync_comets, row);

    // Projection matrix
    C3D_Mtx projection;
    Mtx_PerspStereoTilt(&projection, (45.0f) * M_PI / 180.0f, 400.0f / 240.0f, 0.2f, 70000.0f, iod, 2.0f, false);
    
    // Modelview matrix
    C3D_Mtx modelview;
    Mtx_Identity(&modelview);
    Mtx_Translate(&modelview, 0.0, 0.0, sync_zoom_val, true);
    Mtx_RotateY(&modelview, sync_rotate_val, true);
    Mtx_RotateZ(&modelview, sync_rotate_val * 0.1, true);
    Mtx_RotateX(&modelview, 0.3 * sin(0.3 * sync_rotate_val), true);
    
    C3D_Mtx modelviewSky = modelview;
    Mtx_RotateY(&modelviewSky, 3.0, true);
    
    // Skybox shader
    C3D_BindProgram(&shaderProgramSkybox);
    
    // GPU state for additive blend
    C3D_DepthTest(false, GPU_GEQUAL, GPU_WRITE_COLOR);
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_ONE, GPU_ONE, GPU_ONE, GPU_ONE);
    
    // Uniforms to shader
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocProjectionSkybox, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocModelviewSkybox, &modelviewSky);
    
    // Passthrough texenv and empty lightenv
    C3D_LightEnvInit(&lightEnv);
    C3D_LightEnvBind(&lightEnv);
    
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, 0, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
    
    // Lets draw a skybox
    C3D_TexBind(0, &skybox_tex);
    C3D_CullFace(GPU_CULL_FRONT_CCW);
    C3D_DrawArrays(GPU_TRIANGLES, (numFacesCoolCube) * 3, skyboxVertCount);
//     printf("DRAWARRAYS A.\n");
//     waitForA();
    
    // Normal drawing shader
    C3D_BindProgram(&shaderProgramNormalMapping);
    C3D_TexBind(0, &cubeColTex);
    C3D_TexBind(1, &cubeNormTex);
    
    // Drawing with texture, 2 lights, normal map
    C3D_LightEnvInit(&lightEnv);
    C3D_LightEnvBind(&lightEnv);

    LightLut_Phong(&lutPhong, 100.0);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_D0, GPU_LUTINPUT_LN, false, &lutPhong);
    
    LightLut_FromFunc(&lutShittyFresnel, badFresnel, 1.9, false);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_FR, GPU_LUTINPUT_NV, false, &lutShittyFresnel);
    C3D_LightEnvFresnel(&lightEnv, GPU_PRI_SEC_ALPHA_FRESNEL);
    
    C3D_LightLutDA daLut;
    LightLutDA_Quadratic(&daLut, 1.0, 10.0, 0.01, 0.01);
    
    for(int i = 0; i < 3; i++) {
        float lightscale = 1.0;
        
        float j_scale = 0.01;
        float t = (i / 3.0) * 2.0 * 3.14152 + sync_comets_val + j_scale * 10;
        
        float jr = (float)(TAIL_LEN - 10) / TAIL_LEN;
        double size = -2.775558e-16 + 10.53333*jr - 35.06667*jr*jr + 39.46667*jr*jr*jr- 14.93333*jr*jr*jr*jr;
        
        t = ((int)(t / j_scale)) * j_scale;
        float rad = 11.0 + 0.5 * sin(t + 5.0 * i);
        
        vec3_t pos = sph_sinusoid(rad, 2 + i, 5, t);
        
        if(i == 0) {
          float temp = pos.x;
          pos.x = pos.z;
          pos.z = temp;
        }
        
        if(i == 1) {
          float temp = pos.z;
          pos.z = pos.y;
          pos.y = temp;
        }
        
        // TODO cooler function
//         float t = (i / 3.0) * 2.0 * 3.14152 + sync_comets_val;
//         float x = sin(t) + 2.0 * sin(2.0 * t);
//         float y = cos(t) - 2.0 * cos(2.0 * t);
//         float z = -sin(3.0 * t);
        
        float r = i == 0 ? 1.0 : 0.7;
        float g = i == 1 ? 1.0 : 0.7;
        float b = i == 2 ? 1.0 : 0.7;
        r = g = b = 1.0; // This looks better TODO
        r *= lightscale;
        g *= lightscale;
        b *= lightscale;
        
        C3D_FVec lightVec = FVec4_New(pos.x, pos.y, pos.z, 1.0);
        C3D_LightInit(&light[i], &lightEnv);
        C3D_LightDistAttnEnable(&light[i], true);
        C3D_LightDistAttn(&light[i], &daLut);
        C3D_LightColor(&light[i], r, g, b);
        C3D_LightPosition(&light[i], &lightVec);
    }
//     printf("Lights set\n");
//     waitForA();
    env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_FRAGMENT_PRIMARY_COLOR, 0);
    C3D_TexEnvFunc(env, C3D_RGB, GPU_MODULATE);
    
    env = C3D_GetTexEnv(1);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_PREVIOUS, GPU_FRAGMENT_SECONDARY_COLOR, 0);
    C3D_TexEnvFunc(env, C3D_RGB, GPU_ADD);
    
    C3D_LightEnvBumpMode(&lightEnv, GPU_BUMP_AS_BUMP);
    C3D_LightEnvBumpSel(&lightEnv, 1);

    // GPU state for normal drawing
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
    
    // Uniforms to shader
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocProjection, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocModelview, &modelview);
    
    // Lets draw a cool cube
    C3D_CullFace(GPU_CULL_NONE); 
    C3D_LightEnvMaterial(&lightEnv, &lightMaterial);
//     printf("DRAWARRAYS B pre.\n");
//     waitForA();
    C3D_DrawArrays(GPU_TRIANGLES, 0, numFacesCoolCube * 3);
//     printf("DRAWARRAYS B.\n");
//     waitForA();
//     
    // Bumpmapping off
    C3D_LightEnvBumpSel(&lightEnv, 0);    
    C3D_LightEnvBumpMode(&lightEnv, GPU_BUMP_NOT_USED);
    
//     printf("Bump Off B.\n");
//     waitForA();
    
    // "Just shading" texenv and simple lightenv
    resetShadeEnv();
//     printf("Shade Reset B.\n");
//     waitForA();
//     
//     C3D_TexBind(0, 0);
//     C3D_TexBind(1, 0);
//     printf("TexREmoved.\n");
//     waitForA();
    
    C3D_LightEnvInit(&lightEnv);
    C3D_LightEnvBind(&lightEnv);
    C3D_LightEnvMaterial(&lightEnv, &lightMaterial);

//     printf("LightInitBound.\n");
//     waitForA();
    
    LightLut_Phong(&lutPhong, 100.0);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_D0, GPU_LUTINPUT_LN, false, &lutPhong);
    
    LightLut_FromFunc(&lutShittyFresnel, badFresnel, 1.9, false);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_FR, GPU_LUTINPUT_NV, false, &lutShittyFresnel);
    C3D_LightEnvFresnel(&lightEnv, GPU_PRI_SEC_ALPHA_FRESNEL);
    
//     printf("Set light.\n");
//     waitForA();
    
    C3D_FVec lightVec = FVec4_New(0.0, 0.0, 0.0, 1.0);
    C3D_LightInit(&light[0], &lightEnv);
    C3D_LightColor(&light[0], 1.0, 1.0, 1.0);
    C3D_LightPosition(&light[0], &lightVec);
    
//     printf("Texenv.\n");
//     waitForA();
    
    env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_RGB, GPU_FRAGMENT_PRIMARY_COLOR, GPU_FRAGMENT_SECONDARY_COLOR, 0);
    C3D_TexEnvOpRgb(env, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_RGB, GPU_REPLACE);
    
//     printf("DRAWARRAYS C pre %d %d.\n", cometsStart, cometsVertCount);
//     waitForA();
    
    C3D_DrawArrays(GPU_TRIANGLES, cometsStart, cometsVertCount);
//     printf("DRAWARRAYS C.\n");
//     waitForA();
    
    // And an additive pass to make the text shine
    resetShadeEnv();
    C3D_CullFace(GPU_CULL_BACK_CCW);
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_COLOR);
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
    
    // Passthrough texenv and empty lightenv
    C3D_LightEnvInit(&lightEnv);
    C3D_LightEnvBind(&lightEnv);
    
    env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, 0, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
    
    C3D_TexBind(0, &cubeColAddTex);
    C3D_DrawArrays(GPU_TRIANGLES, 0, numFacesCoolCube * 3);
//     printf("DRAWARRAYS D.\n");
//     waitForA();
}

void effectCoolCubeRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float row) {
    // Update state
    effectCoolCubeUpdate(row);
//     printf("Finished update\n");
//     waitForA();
    // Set up attribute info
    attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0 = position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1 = texcoord
    AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 3); // v2 = normal        
    AttrInfo_AddLoader(attrInfo, 3, GPU_FLOAT, 3); // v3 = tangent
    
    // Start frame
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
//     printf("In frame.\n");
    C3D_RenderTargetClear(targetLeft, C3D_CLEAR_ALL, 0x000000FF, 0);
    C3D_RenderTargetClear(targetRight, C3D_CLEAR_ALL, 0x000000FF, 0);   
    
    // Left eye
//     printf("Drawon.\n");    
    C3D_FrameDrawOn(targetLeft);
//     printf("Call draw.\n");
    effectCoolCubeDraw(-iod, row);
//     printf("FADE.\n");   
    fade();
      
    if(iod > 0.0) {
        // Right eye
        C3D_FrameDrawOn(targetRight);
        effectCoolCubeDraw(iod, row);
        fade();
    }
    
    // Ready to flip
    C3D_FrameEnd(0);
    //gspWaitForPPF();
}

void effectCoolCubeExit() {
//     gspWaitForP3D();
//     gspWaitForPPF();
    
    // Free textures
    printf("tex free\n");
    C3D_TexDelete(&skybox_tex);
    C3D_TexDelete(&cubeColTex);
    C3D_TexDelete(&cubeColAddTex);
    C3D_TexDelete(&cubeNormTex);
    
    // Free allocated memory
    printf("vbo free\n");
    linearFree(vbo);
    linearFree(cubeColPixels);
    linearFree(cubeColAddPixels);
    linearFree(cubeNormPixels);
    
    printf("the problem is outside of this actually\n");
}
