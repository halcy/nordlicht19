// Nordlicht demoparty
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "Tools.h"
#include <vshader_bones_shbin.h>
#include <vshader_skybox_shbin.h>

#include "ModelMotionGuy.h"

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
    { 0.0f, 0.0f, 0.0f }, //ambient
    { 1.0f, 1.0f, 1.0f }, //diffuse
    { 0.4f, 0.4f, 0.4f }, //specular0
    { 0.0f, 0.0f, 0.0f }, //specular1
    { 0.0f, 0.0f, 0.0f }, //emission
};

// Boooones
static int uLocBone[21];

// Proctex
static C3D_ProcTex pt;
static C3D_ProcTexLut pt_map;
static C3D_ProcTexLut pt_noise;
static C3D_ProcTexColorLut pt_clr;

// VBOs
#define VBO_SIZE 70000
static vertex_rigged* vbo;
static C3D_BufInfo* bufInfo;

// Sync
const struct sync_track* sync_zoom;
const struct sync_track* sync_rotate;
const struct sync_track* sync_noise;

void effectDanceInit() {
    // initialize everything here
    sync_zoom = sync_get_track(rocket, "sun2.zoom");
    sync_rotate = sync_get_track(rocket, "sun2.rotate");
    sync_noise = sync_get_track(rocket, "sun2.noise");
    
    // Set up "normal 3D rendering" shader and get uniform locations
    vshader_skybox_dvlb = DVLB_ParseFile((u32*)vshader_skybox_shbin, vshader_skybox_shbin_size);
    vshader_dvlb = DVLB_ParseFile((u32*)vshader_bones_shbin, vshader_bones_shbin_size);
    
    shaderProgramInit(&shaderProgram);
    shaderProgramSetVsh(&shaderProgram, &vshader_dvlb->DVLE[0]);
    
    C3D_BindProgram(&shaderProgram);
    uLocProjection = shaderInstanceGetUniformLocation(shaderProgram.vertexShader, "projection");
    uLocModelview = shaderInstanceGetUniformLocation(shaderProgram.vertexShader, "modelView");
    
    // Bone deform locations
    char boneName[255];
    for(int i = 0; i < 21; i++) {
        sprintf(boneName, "bone%02d", i);
        uLocBone[i] = shaderInstanceGetUniformLocation(shaderProgram.vertexShader, boneName);
        printf("%s -> %d\n", boneName, uLocBone[i]);
    }
    
    shaderProgramInit(&shaderProgramSkybox);
    shaderProgramSetVsh(&shaderProgramSkybox, &vshader_skybox_dvlb->DVLE[0]);
    
    C3D_BindProgram(&shaderProgramSkybox);
    uLocProjectionSkybox = shaderInstanceGetUniformLocation(shaderProgramSkybox.vertexShader, "projection");
    uLocModelviewSkybox = shaderInstanceGetUniformLocation(shaderProgramSkybox.vertexShader, "modelView");
    
    // Allocate VBOs
    vbo = (vertex_rigged*)linearAlloc(sizeof(vertex_rigged) * VBO_SIZE);
    memcpy(vbo, motionGuyVerts, motionGuyNumVerts * sizeof(vertex_rigged));
    
    bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, (void*)vbo, sizeof(vertex_rigged), 5, 0x43210);
    
    // Load statics
    //loadObject2(numFacesSpaceStation, facesSpaceStation, verticesSpaceStation, normalsSpaceStation, texcoordsSpaceStation, &vbo[0]);
    //loadObject2(numFacesSun, facesSun, verticesSun, normalsSun, texcoordsSun, &vbo[numFacesSpaceStation * 3]);
    //skyboxVertCount = buildCube2(&vbo[(numFacesSpaceStation + numFacesSun) * 3], vec3(0, 0, 0), 29500.0, 0.0, 0.0);
    
//     // Load texture for the skybox
//     loadTex3DS(&skybox_tex, &skybox_cube, "romfs:/stars.t3x");
//     C3D_TexSetFilter(&skybox_tex, GPU_LINEAR, GPU_LINEAR);
//     C3D_TexSetWrap(&skybox_tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);       
//     
//     // Load textures for station TODO replace by real model
//     loadTex3DS(&station_tex_col, NULL, "romfs:/tex_spacestation_color.t3x");
//     C3D_TexSetFilter(&station_tex_col, GPU_LINEAR, GPU_NEAREST);
//     C3D_TexSetWrap(&station_tex_col, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);    
//     
//     loadTex3DS(&station_tex_norm, NULL, "romfs:/tex_spacestation_normals.t3x");
//     C3D_TexSetFilter(&station_tex_norm, GPU_LINEAR, GPU_NEAREST);
//     C3D_TexSetWrap(&station_tex_norm, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
}

void effectDanceUpdate(float row) {
    // You can update textures here, but afterwards you have to display transfer and wait for PPF interrupt:
    // GSPGPU_FlushDataCache(screenPixels, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(Pixel));
    // GX_DisplayTransfer((u32*)screenPixels, GX_BUFFER_DIM(SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT), (u32*)screen_tex.data, GX_BUFFER_DIM(SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT), TEXTURE_TRANSFER_FLAGS);
    // gspWaitForPPF();
    
    // Or vertices or any other state ofc.
    // How to get a value from a sync track:
    // Literally nothing lol.
}

void effectDanceDraw(float iod, float row) {
    resetShadeEnv();
    float sync_zoom_val = sync_get_val(sync_zoom, row);
    float sync_rotate_val = sync_get_val(sync_rotate, row);
    float sync_noise_val = sync_get_val(sync_noise, row);

    // Projection matrix
    C3D_Mtx projection;
    Mtx_PerspStereoTilt(&projection, 65.0f * M_PI / 180.0f, 400.0f / 240.0f, 0.2f, 70000.0f, iod, 2.0f, false);
    
    // Modelview matrix
    C3D_Mtx modelview;
    Mtx_Identity(&modelview);
    Mtx_Translate(&modelview, 0.0, 0.0, sync_zoom_val, true);
    //printf("%f %f %f %f\n", modelview.m[12], modelview.m[13], modelview.m[14], modelview.m[15]);
    Mtx_RotateX(&modelview, sync_rotate_val, true);
    //Mtx_RotateZ(&modelview, sync_rotate_val * 0.2, true);
    
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
//     C3D_DrawArrays(GPU_TRIANGLES, (numFacesSpaceStation + numFacesSun) * 3, skyboxVertCount); TODO
    
    // Normal drawing shader
    C3D_BindProgram(&shaderProgram);
    C3D_TexBind(0, &station_tex_col);
    C3D_TexBind(1, &station_tex_norm);
    
    // Drawing with texture, 2 lights, normal map
    C3D_LightEnvInit(&lightEnv);
    C3D_LightEnvBind(&lightEnv);

    LightLut_Phong(&lutPhong, 100.0);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_D0, GPU_LUTINPUT_LN, false, &lutPhong);
    
//     LightLut_FromFunc(&lutShittyFresnel, badFresnel, 1.9, false);
//     C3D_LightEnvLut(&lightEnv, GPU_LUT_FR, GPU_LUTINPUT_NV, false, &lutShittyFresnel);
//     C3D_LightEnvFresnel(&lightEnv, GPU_PRI_SEC_ALPHA_FRESNEL);
    
    C3D_FVec lightVec = FVec4_New(1.0, 3.0, 3.0, 1.0);
    C3D_LightInit(&light, &lightEnv);
    C3D_LightColor(&light, 0.8, 0.8, 1.0);
    C3D_LightPosition(&light, &lightVec);

    env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_FRAGMENT_PRIMARY_COLOR, GPU_FRAGMENT_SECONDARY_COLOR, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_ADD);

    // GPU state for normal drawing
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
    //C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_ONE, GPU_ONE, GPU_ONE, GPU_ONE);
    
    // Uniforms to shader
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocProjection, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocModelview, &modelview);
    
    // Bones to shader TODO can we not do this twice? probably unvaoivable though unless I want to reverse draw order for right eye
    int frame = (int)sync_noise_val;
    for(int i = 0; i < 21; i++) {
        C3D_Mtx boneMat;
        Mtx_Identity(&boneMat);
        for(int j = 0; j < 4 * 3; j++) {
            boneMat.m[j] = motionGuyAnim[frame][i][j];
        }
        
//         printf("%d -> %f %f\n", uLocBone[i], boneMat.m[0], boneMat.m[1]);
        C3D_FVUnifMtx3x4(GPU_VERTEX_SHADER, uLocBone[i], &boneMat);
    }
    
    // Lets draw a space station
    C3D_CullFace(GPU_CULL_NONE); 
    C3D_LightEnvMaterial(&lightEnv, &lightMaterial);
    C3D_DrawArrays(GPU_TRIANGLES, 0, motionGuyNumVerts);
}

void effectDanceRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float row) {
    // Update state
    effectDanceUpdate(row);
    
    // Set up attribute info
    attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0 = position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 4); // v1 = bone indices
    AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 4); // v2 = bone weights
    AttrInfo_AddLoader(attrInfo, 3, GPU_FLOAT, 3); // v3 = normal        
    AttrInfo_AddLoader(attrInfo, 4, GPU_FLOAT, 2); // v4 = texcoords
    
    // Start frame
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C3D_RenderTargetClear(targetLeft, C3D_CLEAR_ALL, 0xFF339922, 0);
    C3D_RenderTargetClear(targetRight, C3D_CLEAR_ALL, 0xFF339922, 0);   
    
    // Left eye
    C3D_FrameDrawOn(targetLeft);
    effectDanceDraw(-iod, row);
    fade();
      
    if(iod > 0.0) {
        // Right eye
        C3D_FrameDrawOn(targetRight);
        effectDanceDraw(iod, row);
        fade();
    }
    
    // Ready to flip
    C3D_FrameEnd(0);
    //gspWaitForPPF();
}

void effectDanceExit() {
    gspWaitForPPF();
    
    // Free textures
    printf("tex free\n");
    C3D_TexDelete(&skybox_tex);
    C3D_TexDelete(&station_tex_col);
    C3D_TexDelete(&station_tex_norm);
    
    // Free allocated memory
    printf("vbo free\n");
    linearFree(vbo);

    // Free the shaders
    printf("shaders free\n");
    shaderProgramFree(&shaderProgram);
    DVLB_Free(vshader_dvlb);
    
    shaderProgramFree(&shaderProgramSkybox);
    DVLB_Free(vshader_skybox_dvlb);
    
    printf("the problem is outside of this actually\n");
}
