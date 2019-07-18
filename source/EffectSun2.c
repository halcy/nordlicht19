// Nordlicht demoparty
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "Tools.h"

#include "ModelSpaceStation.h"
#include "ModelSun.h"

// Shader / textures
static C3D_AttrInfo* attrInfo;

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
    { 0.0f, 0.0f, 0.0f }, //ambient
    { 1.0f, 1.0f, 1.0f }, //diffuse
    { 0.4f, 0.4f, 0.4f }, //specular0
    { 0.0f, 0.0f, 0.0f }, //specular1
    { 0.0f, 0.0f, 0.0f }, //emission
};

// Proctex
static C3D_ProcTex pt;
static C3D_ProcTexLut pt_map;
static C3D_ProcTexLut pt_noise;
static C3D_ProcTexColorLut pt_clr;

// VBOs
#define VBO_SIZE 100000
static vertex2* vbo;
static C3D_BufInfo* bufInfo;

// Sync
const struct sync_track* sync_zoom;
const struct sync_track* sync_rotate;
const struct sync_track* sync_rotate2;
const struct sync_track* sync_noise;
const struct sync_track* sync_fov;

void effectSun2Init() {
    // initialize everything here
    sync_zoom = sync_get_track(rocket, "sun2.zoom");
    sync_rotate = sync_get_track(rocket, "sun2.rotate");
    sync_rotate2 = sync_get_track(rocket, "sun2.rotate2");
    sync_noise = sync_get_track(rocket, "sun2.noise");
    sync_fov = sync_get_track(rocket, "sun2.fov");
    
    // Set up "normal 3D rendering" shader and get uniform locations
    C3D_BindProgram(&shaderProgramNormalMapping);
    uLocProjection = shaderInstanceGetUniformLocation(shaderProgramNormalMapping.vertexShader, "projection");
    uLocModelview = shaderInstanceGetUniformLocation(shaderProgramNormalMapping.vertexShader, "modelView");
    
    C3D_BindProgram(&shaderProgramSkybox);
    uLocProjectionSkybox = shaderInstanceGetUniformLocation(shaderProgramSkybox.vertexShader, "projection");
    uLocModelviewSkybox = shaderInstanceGetUniformLocation(shaderProgramSkybox.vertexShader, "modelView");
    
    // Allocate VBOs
    vbo = (vertex2*)linearAlloc(sizeof(vertex2) * VBO_SIZE);
    
    bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, (void*)vbo, sizeof(vertex2), 4, 0x3210);
    
    // Load statics
    loadObject2(numFacesSpaceStation, facesSpaceStation, verticesSpaceStation, normalsSpaceStation, texcoordsSpaceStation, &vbo[0]);
    loadObject2(numFacesSun, facesSun, verticesSun, normalsSun, texcoordsSun, &vbo[numFacesSpaceStation * 3]);
    skyboxVertCount = buildCube2(&vbo[(numFacesSpaceStation + numFacesSun) * 3], vec3(0, 0, 0), 29500.0, 0.0, 0.0);
    
    // Load texture for the skybox
    loadTex3DS(&skybox_tex, &skybox_cube, "romfs:/stars.bin");
    C3D_TexSetFilter(&skybox_tex, GPU_LINEAR, GPU_LINEAR);
    C3D_TexSetWrap(&skybox_tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);       
    
    // Load textures for station
    loadTex3DS(&station_tex_col, NULL, "romfs:/tex_spacestation_color.bin");
    C3D_TexSetFilter(&station_tex_col, GPU_LINEAR, GPU_NEAREST);
    C3D_TexSetWrap(&station_tex_col, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);    
    
    loadTex3DS(&station_tex_norm, NULL, "romfs:/tex_spacestation_normals.bin");
    C3D_TexSetFilter(&station_tex_norm, GPU_LINEAR, GPU_NEAREST);
    C3D_TexSetWrap(&station_tex_norm, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
    
    // Proctex init
    C3D_ProcTexInit(&pt, 0, 5);
    C3D_ProcTexClamp(&pt, GPU_PT_CLAMP_TO_EDGE, GPU_PT_CLAMP_TO_EDGE);
    C3D_ProcTexCombiner(&pt, false, GPU_PT_SQRT2, GPU_PT_SQRT2);
    C3D_ProcTexFilter(&pt, GPU_PT_LINEAR_MIP_LINEAR);
    C3D_ProcTexBind(0, &pt);

    float data[129];
    int i;
    for (i = 0; i <= 128; i ++)
    {
        float x = i/128.0f;
        data[i] = fabsf(sinf(C3D_Angle(2*(x+0.125f))));
    }
    ProcTexLut_FromArray(&pt_map, data);
    C3D_ProcTexLutBind(GPU_LUT_RGBMAP, &pt_map);

    // Noise smooth step equation
    for (i = 0; i <= 128; i ++)
    {
        float x = i / 128.0f;
        data[i] = x*x*(3-2*x);
    }
    ProcTexLut_FromArray(&pt_noise, data);
    C3D_ProcTexLutBind(GPU_LUT_NOISE, &pt_noise);

    u32 colors[5];
    colors[0] = 0xFFefe87f;
    colors[1] = 0xFFfffeea;
    colors[2] = 0xFFfffeea;
    colors[3] = 0xFFfffeea;
    colors[4] = 0xFFfffeea;

    ProcTexColorLut_Write(&pt_clr, colors, 0, 5);
    C3D_ProcTexColorLutBind(&pt_clr);
}

void effectSun2Update(float row) {
    // You can update textures here, but afterwards you have to display transfer and wait for PPF interrupt:
    // GSPGPU_FlushDataCache(screenPixels, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(Pixel));
    // GX_DisplayTransfer((u32*)screenPixels, GX_BUFFER_DIM(SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT), (u32*)screen_tex.data, GX_BUFFER_DIM(SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT), TEXTURE_TRANSFER_FLAGS);
    // gspWaitForPPF();
    
    // Or vertices or any other state ofc.
    // How to get a value from a sync track:
    // Literally nothing lol. should move proctex probably
}

void effectSun2Draw(float iod, float row) {
    resetShadeEnv();
    float sync_zoom_val = sync_get_val(sync_zoom, row);
    float sync_rotate_val = sync_get_val(sync_rotate, row);
    float sync_rotate2_val = sync_get_val(sync_rotate2, row);
    float sync_noise_val = sync_get_val(sync_noise, row);
    float sync_fov_val = sync_get_val(sync_fov, row);

    // Projection matrix
    C3D_Mtx projection;
    Mtx_PerspStereoTilt(&projection, (65.0f + sync_fov_val) * M_PI / 180.0f, 400.0f / 240.0f, 0.2f, 70000.0f, iod, 2.0f, false);
    
    // Modelview matrix
    C3D_Mtx modelview;
    Mtx_Identity(&modelview);
    Mtx_RotateY(&modelview, sync_rotate2_val, true);
    Mtx_Translate(&modelview, 0.0, 0.0, sync_zoom_val, true);
    Mtx_RotateY(&modelview, sync_rotate_val, true);
    
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
    C3D_DrawArrays(GPU_TRIANGLES, (numFacesSpaceStation + numFacesSun) * 3, skyboxVertCount);
    
    // Normal drawing shader
    C3D_BindProgram(&shaderProgramNormalMapping);
    C3D_TexBind(0, &station_tex_col);
    C3D_TexBind(1, &station_tex_norm);
    
    // Drawing with texture, 2 lights, normal map
    C3D_LightEnvInit(&lightEnv);
    C3D_LightEnvBind(&lightEnv);

    LightLut_Phong(&lutPhong, 100.0);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_D0, GPU_LUTINPUT_LN, false, &lutPhong);
    
    LightLut_FromFunc(&lutShittyFresnel, badFresnel, 1.9, false);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_FR, GPU_LUTINPUT_NV, false, &lutShittyFresnel);
    C3D_LightEnvFresnel(&lightEnv, GPU_PRI_SEC_ALPHA_FRESNEL);
    
    C3D_FVec lightVec = FVec4_New(1.0, 3.0, 3.0, 1.0);
    C3D_LightInit(&light, &lightEnv);
    C3D_LightColor(&light, 0.8, 0.8, 1.0);
    C3D_LightPosition(&light, &lightVec);

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
    
    // Lets draw a space station
    C3D_CullFace(GPU_CULL_BACK_CCW); 
    C3D_LightEnvMaterial(&lightEnv, &lightMaterial);
    if(sync_rotate2_val < 0.1) {
        C3D_DrawArrays(GPU_TRIANGLES, 0, numFacesSpaceStation * 3);
    }
    
    // Proc tex update
    C3D_ProcTexNoiseCoefs(&pt, C3D_ProcTex_U, 0.125f, 200, 1.0 * 0.0001);
    C3D_ProcTexNoiseCoefs(&pt, C3D_ProcTex_V, 0.125f, 200, sync_noise_val * 0.00005);
    C3D_ProcTexBind(0, &pt);
    
    //resetShadeEnv();
    env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE3, GPU_FRAGMENT_PRIMARY_COLOR, 0);
    C3D_TexEnvFunc(env, C3D_RGB, GPU_REPLACE);
    
    // Lets draw the sun
    if(sync_rotate2_val < 1.7) {
        C3D_DrawArrays(GPU_TRIANGLES, numFacesSpaceStation * 3, numFacesSun * 3);
    }
    
    // Bumpmapping off
    C3D_LightEnvBumpSel(&lightEnv, 0);    
    C3D_LightEnvBumpMode(&lightEnv, GPU_BUMP_NOT_USED);
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
    AttrInfo_AddLoader(attrInfo, 3, GPU_FLOAT, 3); // v3 = tangent
    
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
    //gspWaitForPPF();
}

void effectSun2Exit() { 
//     gspWaitForP3D();
//     gspWaitForPPF();
    
    // Free allocated memory
    printf("vbo free\n");
    linearFree(vbo);

    // Free textures
    printf("tex free\n");
    C3D_TexDelete(&skybox_tex);
    C3D_TexDelete(&station_tex_col);
    C3D_TexDelete(&station_tex_norm);
    
    printf("the problem is outside of this actually\n");
}
