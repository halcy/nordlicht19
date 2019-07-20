// Nordlicht demoparty
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "Tools.h"

#include "ModelMotionGuy.h"
#include "ModelPlatform.h"
#include "ModelCylinder.h"

// Shader / textures
static C3D_AttrInfo* attrInfo;

static int uLocProjection;
static int uLocModelview;
static int uLocProjectionSkybox;
static int uLocModelviewSkybox;

static C3D_Tex guy_tex;
static C3D_Tex plat_tex;
static C3D_Tex screen_tex;

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

// Cylinder
static Pixel* screenPixels;
static Bitmap screen;

// VBOs
#define VBO_SIZE 30000
static vertex_rigged* vbo;
static C3D_BufInfo* bufInfo;

// Sync
const struct sync_track* sync_zoom;
const struct sync_track* sync_rotate;
const struct sync_track* sync_movement;
const struct sync_track* sync_stars;
const struct sync_track* sync_starmax;

// Stars!
#define NUM_STARS 300
static vec2_t starPos[NUM_STARS];
float starBright[NUM_STARS];

FILE *danceFile;

void effectDanceInit() {
    // Load a dance
    danceFile = fopen("romfs:/dance_anim.bin", "rb");
        
    // initialize everything here
    sync_zoom = sync_get_track(rocket, "guy.zoom");
    sync_rotate = sync_get_track(rocket, "guy.rotate");
    sync_movement = sync_get_track(rocket, "guy.anim");
    sync_stars = sync_get_track(rocket, "guy.stars");
    sync_starmax = sync_get_track(rocket, "guy.starmax");
    
    // Stars
    for(int i = 0; i < NUM_STARS; i++) {
        starPos[i].x = (float)(rand() % 256);
        starPos[i].y = (float)(rand() % 256);
        starBright[i] = (float)(rand() % 256) / 256.0;
    }
    
    // Set up "normal 3D rendering" shader and get uniform locations
    C3D_BindProgram(&shaderProgramBones);
    uLocProjection = shaderInstanceGetUniformLocation(shaderProgramBones.vertexShader, "projection");
    uLocModelview = shaderInstanceGetUniformLocation(shaderProgramBones.vertexShader, "modelView");
    
    // Bone deform locations
    char boneName[255];
    for(int i = 0; i < 21; i++) {
        sprintf(boneName, "bone%02d", i);
        uLocBone[i] = shaderInstanceGetUniformLocation(shaderProgramBones.vertexShader, boneName);
    }
    
    C3D_BindProgram(&shaderProgramSkybox);
    uLocProjectionSkybox = shaderInstanceGetUniformLocation(shaderProgramSkybox.vertexShader, "projection");
    uLocModelviewSkybox = shaderInstanceGetUniformLocation(shaderProgramSkybox.vertexShader, "modelView");
    
    // Allocate VBOs
    vbo = (vertex_rigged*)linearAlloc(sizeof(vertex_rigged) * VBO_SIZE);
    memcpy(vbo, motionGuyVerts, motionGuyNumVerts * sizeof(vertex_rigged));
    memcpy(&vbo[motionGuyNumVerts], platformVerts, platformNumVerts * sizeof(vertex_rigged));
    memcpy(&vbo[motionGuyNumVerts + platformNumVerts], cylinderVerts, cylinderNumVerts * sizeof(vertex_rigged));
    
    bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, (void*)vbo, sizeof(vertex_rigged), 5, 0x43210);
        
    // Load textures for station
    loadTex3DS(&guy_tex, NULL, "romfs:/guy.t3x");
    C3D_TexSetFilter(&guy_tex, GPU_LINEAR, GPU_NEAREST);
    C3D_TexSetWrap(&guy_tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE); 
    
    loadTex3DS(&plat_tex, NULL, "romfs:/platform.t3x");
    C3D_TexSetFilter(&plat_tex, GPU_LINEAR, GPU_NEAREST);
    C3D_TexSetWrap(&plat_tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
    
    screenPixels = (Pixel*)linearAlloc(256 * 256 * sizeof(Pixel));
    InitialiseBitmap(&screen, 256, 256, BytesPerRowForWidth(256), screenPixels);
    C3D_TexInit(&screen_tex, 256, 256, GPU_RGBA8);
    C3D_TexSetFilter(&screen_tex, GPU_LINEAR, GPU_LINEAR);
    C3D_TexSetWrap(&screen_tex, GPU_REPEAT, GPU_REPEAT);    
}

void effectDanceUpdate(float row) {
    float sync_stars_val = sync_get_val(sync_stars, row);
    float sync_starmax_val = sync_get_val(sync_starmax, row);
    int max_star = min(sync_starmax_val, NUM_STARS);
    
    // Render some 2D stuff
    FillBitmap(&screen, RGBAf(0.05, 0.05, 0.05, 1.0));    
    for(int i = 0; i < max_star; i++) {
        float x = fmod(starPos[i].x - sync_stars_val, 256.0);
        if(x < 0.0) {
            x = x + 256.0;
        }
        float starCol = 0.2 + starBright[i] * 0.75;
        DrawFilledCircle(&screen, x, starPos[i].y + 5, 2, RGBAf(starCol, starCol, starCol, 1.0));
    }
    
    GSPGPU_FlushDataCache(screenPixels, 256 * 256 * sizeof(Pixel));
    GX_DisplayTransfer((u32*)screenPixels, GX_BUFFER_DIM(256, 256), (u32*)screen_tex.data, GX_BUFFER_DIM(256, 256), TEXTURE_TRANSFER_FLAGS);
    gspWaitForPPF();
}

void effectDanceDraw(float iod, float row) {
    resetShadeEnv();
    float sync_zoom_val = sync_get_val(sync_zoom, row);
    float sync_rotate_val = sync_get_val(sync_rotate, row);
    float sync_movement_val = sync_get_val(sync_movement, row);

    // Projection matrix
    C3D_Mtx projection;
    Mtx_PerspStereoTilt(&projection, 65.0f * M_PI / 180.0f, 400.0f / 240.0f, 0.2f, 10000000.0f, iod, 2.0f, false);
    
    // Modelview matrix
    C3D_Mtx modelview;
    Mtx_Identity(&modelview);
    Mtx_Translate(&modelview, 0.0, 0.0, sync_zoom_val, true);
    //printf("%f %f %f %f\n", modelview.m[12], modelview.m[13], modelview.m[14], modelview.m[15]);
    Mtx_RotateX(&modelview, -1.0, true);
    Mtx_RotateZ(&modelview, sync_rotate_val * 0.2, true);
    
    C3D_Mtx modelviewSky = modelview;
    Mtx_RotateY(&modelviewSky, 3.0, true);
    
    // Normal drawing shader
    C3D_BindProgram(&shaderProgramBones);
    
    // Uniforms to shader
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocProjection, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocModelview, &modelview);
    
    // GPU state for additive blend
    C3D_DepthTest(false, GPU_GEQUAL, GPU_WRITE_COLOR);
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_ONE, GPU_ONE, GPU_ONE, GPU_ONE);
    
    // Passthrough texenv and empty lightenv
    C3D_LightEnvInit(&lightEnv);
    C3D_LightEnvBind(&lightEnv);
    
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, 0, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
    
    // Surrounding cylinder
    C3D_TexBind(0, &screen_tex);
    C3D_Mtx boneMat;
    for(int i = 0; i < 16; i++) {
        boneMat.m[i] = 0.0;
    }
    boneMat.m[0] = 1.0;
    boneMat.m[5] = 1.0;
    boneMat.m[10] = 1.0;
    C3D_CullFace(GPU_CULL_NONE);
    C3D_FVUnifMtx3x4(GPU_VERTEX_SHADER, uLocBone[0], &boneMat);
    C3D_DrawArrays(GPU_TRIANGLES, motionGuyNumVerts + platformNumVerts, cylinderNumVerts);
    
    // Drawing with texture
    C3D_LightEnvInit(&lightEnv);
    C3D_LightEnvBind(&lightEnv);
    
    LightLut_Phong(&lutPhong, 100.0);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_D0, GPU_LUTINPUT_LN, false, &lutPhong);
    
    LightLut_FromFunc(&lutShittyFresnel, badFresnel, 1.9, false);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_FR, GPU_LUTINPUT_NV, false, &lutShittyFresnel);
    C3D_LightEnvFresnel(&lightEnv, GPU_PRI_SEC_ALPHA_FRESNEL);
    
    C3D_FVec lightVec = FVec4_New(1.0, 3.0, 3.0, 1.0);
    C3D_LightInit(&light, &lightEnv);
    C3D_LightColor(&light, 2.0, 2.0, 2.0);
    C3D_LightPosition(&light, &lightVec);

//     env = C3D_GetTexEnv(0);
//     C3D_TexEnvInit(env);
//     C3D_TexEnvSrc(env, C3D_Both, GPU_FRAGMENT_PRIMARY_COLOR, GPU_FRAGMENT_SECONDARY_COLOR, 0);
//     C3D_TexEnvFunc(env, C3D_Both, GPU_ADD);
    
    env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_FRAGMENT_PRIMARY_COLOR, 0);
    C3D_TexEnvFunc(env, C3D_RGB, GPU_MODULATE);
    
    env = C3D_GetTexEnv(1);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_PREVIOUS, GPU_FRAGMENT_SECONDARY_COLOR, 0);
    C3D_TexEnvFunc(env, C3D_RGB, GPU_ADD);

    // GPU state for normal drawing
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
    //C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_ONE, GPU_ONE, GPU_ONE, GPU_ONE);
    
    // Bones to shader TODO can we not do this twice? probably unvaoivable though unless I want to reverse draw order for right eye
    int frame = min(max(0, (int)sync_movement_val), motionGuyNumFrames);
    for(int i = 0; i < 21; i++) {
        Mtx_Identity(&boneMat);
        /*for(int j = 0; j < 4 * 3; j++) {
            
            // boneMat.m[j] = motionGuyAnim[frame][i][j];
        }*/
        fseek(danceFile, ((frame * 21 * 12) + (i * 12)) * sizeof(float), SEEK_SET);
        fread(boneMat.m, 12 * sizeof(float), 1, danceFile);
//         printf("%f %f %f %f\n", boneMat.m[0], boneMat.m[1], boneMat.m[2], boneMat.m[3]);
        C3D_FVUnifMtx3x4(GPU_VERTEX_SHADER, uLocBone[i], &boneMat);
    } 
    
    // Draw the guy
    C3D_TexBind(0, &guy_tex);
    C3D_CullFace(GPU_CULL_BACK_CCW); 
    C3D_LightEnvMaterial(&lightEnv, &lightMaterial);
    C3D_DrawArrays(GPU_TRIANGLES, 0, motionGuyNumVerts);
    
    // Platform    
    C3D_TexBind(0, &plat_tex);
    for(int i = 0; i < 16; i++) {
        boneMat.m[i] = 0.0;
    }
    boneMat.m[0] = 1.0;
    boneMat.m[5] = 1.0;
    boneMat.m[10] = 1.0;
    C3D_FVUnifMtx3x4(GPU_VERTEX_SHADER, uLocBone[0], &boneMat);
    C3D_DrawArrays(GPU_TRIANGLES, motionGuyNumVerts, platformNumVerts);
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
    C3D_RenderTargetClear(targetLeft, C3D_CLEAR_ALL, 0x000000FF, 0);
    C3D_RenderTargetClear(targetRight, C3D_CLEAR_ALL, 0x000000FF, 0);   

    // Left eye
    C3D_FrameDrawOn(targetLeft);
    effectDanceDraw(-iod, row);
      
    if(iod > 0.0) {
        // Right eye
        C3D_FrameDrawOn(targetRight);
        effectDanceDraw(iod, row);
        fade();
    }
    
    C3D_FrameDrawOn(targetLeft);    
    fade();
    
    // Ready to flip
    C3D_FrameEnd(0);
    //gspWaitForPPF();
}

void effectDanceExit() {
//     gspWaitForP3D();
//     gspWaitForPPF();
//     
    // Free allocated memory
//     printf("vbo free\n");
    linearFree(vbo);
    linearFree(screenPixels);
    
    // Free textures
//     printf("tex free\n");
    C3D_TexDelete(&plat_tex);
    C3D_TexDelete(&guy_tex);
    C3D_TexDelete(&screen_tex);
    
    fclose(danceFile);
    
//     printf("the problem is outside of this actually\n");
}
