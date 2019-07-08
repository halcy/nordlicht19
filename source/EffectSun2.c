// Nordlicht demoparty
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "Tools.h"
#include <vshader_shbin.h>
#include <vshader_skybox_shbin.h>

#include "ModelSpaceStation.h"

// Shader / textures
static C3D_AttrInfo* attrInfo;

static DVLB_s* vshader_dvlb;
static DVLB_s* vshader_skybox_dvlb;
static shaderProgram_s shaderProgram;
static shaderProgram_s shaderProgramSkybox;

static int uLocProjection;
static int uLocModelview;
static int uLocProjectionSkybox;
static int uLocModelviewSkybox;

static C3D_Tex skybox_tex;
static C3D_TexCube skybox_cube;
int skyboxVertCount;

static C3D_Tex station_tex_col;
static C3D_Tex station_tex_norm;

// Lighting
static C3D_LightEnv lightEnv;
static C3D_Light light;
static C3D_LightLut lutShittyFresnel;
static C3D_LightLut lutPhong;

static const C3D_Material lightMaterial = {
    { 0.1f, 0.1f, 0.1f }, //ambient
    { 0.8f, 0.8f, 0.8f }, //diffuse
    { 0.5f, 0.4f, 0.2f }, //specular0
    { 0.0f, 0.0f, 0.0f }, //specular1
    { 0.0f, 0.0f, 0.0f }, //emission
};

static const C3D_Material lightMaterialDark = {
    { 0.1f, 0.1f, 0.1f }, //ambient
    { 0.5f, 0.5f, 0.5f }, //diffuse
    { 0.5f, 0.4f, 0.2f }, //specular0
    { 0.0f, 0.0f, 0.0f }, //specular1
    { 0.0f, 0.0f, 0.0f }, //emission
};

// VBOs
#define VBO_SIZE 50000
static vertex* vbo;
static C3D_BufInfo* bufInfo;

const struct sync_track* sync_testval;

void effectSun2Init() {
    // initialize everything here
    // Example of how to get a sync track
    sync_testval = sync_get_track(rocket, "template.testval");
    
    // Set up "normal 3D rendering" shader and get uniform locations
    vshader_skybox_dvlb = DVLB_ParseFile((u32*)vshader_skybox_shbin, vshader_skybox_shbin_size);
    vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
    
    shaderProgramInit(&shaderProgram);
    shaderProgramSetVsh(&shaderProgram, &vshader_dvlb->DVLE[0]);
    
    C3D_BindProgram(&shaderProgram);
    uLocProjection = shaderInstanceGetUniformLocation(shaderProgram.vertexShader, "projection");
    uLocModelview = shaderInstanceGetUniformLocation(shaderProgram.vertexShader, "modelView");
    
    shaderProgramInit(&shaderProgramSkybox);
    shaderProgramSetVsh(&shaderProgramSkybox, &vshader_skybox_dvlb->DVLE[0]);
    
    C3D_BindProgram(&shaderProgramSkybox);
    uLocProjectionSkybox = shaderInstanceGetUniformLocation(shaderProgramSkybox.vertexShader, "projection");
    uLocModelviewSkybox = shaderInstanceGetUniformLocation(shaderProgramSkybox.vertexShader, "modelView");
    
    // Allocate VBOs
    vbo = (vertex*)linearAlloc(sizeof(vertex) * VBO_SIZE);
    
    bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, (void*)vbo, sizeof(vertex), 3, 0x210);
    
    // Load statics
    loadObject2(numFacesSpaceStation, facesSpaceStation, verticesSpaceStation, normalsSpaceStation, texcoordsSpaceStation, &vbo[0]);
    skyboxVertCount = buildCube(&vbo[numFacesSpaceStation * 3], vec3(0, 0, 0), 2000.0, 0.0, 0.0);
    
    // Load texture for the skybox
    loadTex3DS(&skybox_tex, &skybox_cube, "romfs:/stars.t3x");
    C3D_TexSetFilter(&skybox_tex, GPU_LINEAR, GPU_LINEAR);
    C3D_TexSetWrap(&skybox_tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);       
    
    // Load textures for station
    loadTex3DS(&station_tex_col, NULL, "romfs:/tex_spacestation_color.t3x");
    C3D_TexSetFilter(&station_tex_col, GPU_LINEAR, GPU_LINEAR);
    C3D_TexSetWrap(&station_tex_col, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);    
    
    loadTex3DS(&station_tex_norm, NULL, "romfs:/tex_spacestation_normals.t3x");
    C3D_TexSetFilter(&station_tex_norm, GPU_LINEAR, GPU_LINEAR);
    C3D_TexSetWrap(&station_tex_norm, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);       
}

void effectSun2Update(float row) {
    // You can update textures here, but afterwards you have to display transfer and wait for PPF interrupt:
    // GSPGPU_FlushDataCache(screenPixels, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(Pixel));
    // GX_DisplayTransfer((u32*)screenPixels, GX_BUFFER_DIM(SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT), (u32*)screen_tex.data, GX_BUFFER_DIM(SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT), TEXTURE_TRANSFER_FLAGS);
    // gspWaitForPPF();
    
    // Or vertices or any other state ofc.
    // How to get a value from a sync track:
    float testval = sync_get_val(sync_testval, row);
}

void effectSun2Draw(float iod, float row) {    
    // Projection matrix
    C3D_Mtx projection;
    Mtx_PerspStereoTilt(&projection, 65.0f * M_PI / 180.0f, 400.0f / 240.0f, 0.2f, 5000.0f, iod, 2.0f, false);
    
    // Modelview matrix
    C3D_Mtx modelview;
    Mtx_Identity(&modelview);
    Mtx_Translate(&modelview, 0.0, 0.0, -0.5, true);
    Mtx_RotateY(&modelview, row * 0.01, true);
    
    // Skybox shader
    C3D_BindProgram(&shaderProgramSkybox);
    
    // GPU state for additive blend
    C3D_DepthTest(false, GPU_GEQUAL, GPU_WRITE_COLOR);
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_ONE, GPU_ONE, GPU_ONE, GPU_ONE);
    
    // Uniforms to shader
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocProjectionSkybox, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocModelviewSkybox, &modelview);
    
    // Passthrough texenv and empty lightenv
    C3D_LightEnvBind(0);    
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, 0, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
    
    // Lets draw a skybox
    C3D_TexBind(0, &skybox_tex);
    C3D_CullFace(GPU_CULL_FRONT_CCW);
    C3D_DrawArrays(GPU_TRIANGLES, numFacesSpaceStation * 3, skyboxVertCount);    
    
    // Normal drawing shader
    C3D_BindProgram(&shaderProgram);
    C3D_TexBind(0, &station_tex_col);
    
    // GPU state for normal drawing
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
    
    // Uniforms to shader
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocProjection, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocModelview, &modelview);
    
    // Lets draw a space station
    C3D_CullFace(GPU_CULL_NONE);
    C3D_LightEnvMaterial(&lightEnv, &lightMaterialDark);
    C3D_DrawArrays(GPU_TRIANGLES, 0, numFacesSpaceStation * 3);
}

void effectSun2Render(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float row) {
    // Update state
    effectSun2Update(row);
    
    // Set up attribute info
    attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0 = position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1 = texcoord
    AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 3); // v2 = normal        
    
    // Start frame
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C3D_RenderTargetClear(targetLeft, C3D_CLEAR_ALL, 0, 0);
    C3D_RenderTargetClear(targetRight, C3D_CLEAR_ALL, 0, 0);   
    
    // Left eye
    C3D_FrameDrawOn(targetLeft);
    effectSun2Draw(-iod, row);
    fade();
    
    if(iod > 0.0) {
        // Right eye
        C3D_FrameDrawOn(targetRight);
        effectSun2Draw(iod, row);
        fade();
    }
    
    // Ready to flip
    C3D_FrameEnd(0);
}

void effectSun2Exit() {
    // Free textures
    C3D_TexDelete(&skybox_tex);
    
    // Free allocated memory
    linearFree(vbo);

    // Free the shaders
    shaderProgramFree(&shaderProgram);
    DVLB_Free(vshader_dvlb);
    
    shaderProgramFree(&shaderProgramSkybox);
    DVLB_Free(vshader_skybox_dvlb);
}
