// Nordlicht demoparty
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "Tools.h"
#include <vshader_shbin.h>
#include "logo_bin.h"
#include "Perlin.h"

static DVLB_s* vshader_dvlb;
static shaderProgram_s program;

static int uLocProjection;
static int uLocModelview;
static C3D_Mtx projection;

static Pixel* screenPixels;
static Bitmap screen;
static C3D_Tex screen_tex;
static C3D_Tex logo_tex;

static C3D_LightEnv lightEnv;
static C3D_Light light;
static C3D_LightLut lutPhong;

#include "Font.h"
#include "MonoFont.h"

extern Font OL16Font; 

#define SCROLLERTEXT "EVERY MORNING I WAKE UP AND OPEN PALM SLAM A VHS INTO THE SLOT. ITS CHRONICLES OF RIDDICK AND RIGHT THEN AND THERE I START DOING THE MOVES ALONGSIDE WITH THE MAIN CHARACTER, RIDDICK. I DO EVERY MOVE AND I DO EVERY MOVE HARD. MAKIN WHOOSHING SOUNDS WHEN I SLAM DOWN SOME NECRO BASTARDS OR EVEN WHEN I MESS UP TECHNIQUE. NOT MANY CAN SAY THEY ESCAPED THE GALAXY’S MOST DANGEROUS PRISON. I CAN. I SAY IT AND I SAY IT OUTLOUD EVERYDAY TO PEOPLE IN MY COLLEGE CLASS AND ALL THEY DO IS PROVE PEOPLE IN COLLEGE CLASS CAN STILL BE IMMATURE JERKS. AND IVE LEARNED ALL THE LINES AND IVE LEARNED HOW TO MAKE MYSELF AND MY APARTMENT LESS LONELY BY SHOUTING EM ALL. 2 HOURS INCLUDING WIND DOWN EVERY MORNING."

static Pixel* scrollPixels;
static Bitmap scroller;
static C3D_Tex scroll_tex;

const struct sync_track* sync_scroll_pos;
const struct sync_track* sync_bg_pos;

static const C3D_Material lightMaterial = {
    { 0.1f, 0.1f, 0.1f }, //ambient
    { 0.2f, 0.8f, 0.4f }, //diffuse
    { 0.8f, 0.8f, 0.9f }, //specular0
    { 0.0f, 0.0f, 0.0f }, //specular1
    { 0.0f, 0.1f, 0.0f }, //emission
};

static vertex* tunnelVBO;
int32_t vertCount;
#define MAX_VERTS 30000

// EXTREMELY simple scroller
void effectTunnelInit() {
    // Get rocket track
    sync_scroll_pos = sync_get_track(rocket, "scroller.pos");
    sync_bg_pos = sync_get_track(rocket, "scroller.bgpos");
    
    // Load default shader
    vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
    shaderProgramInit(&program);
    shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
    
    // Scroller texture
    C3D_TexInit(&scroll_tex, 512, 512, GPU_RGBA8);
    scrollPixels = (Pixel*)linearAlloc(512 * 512 * sizeof(Pixel));
    InitialiseBitmap(&scroller, 512, 512, BytesPerRowForWidth(512), scrollPixels);
    
    // BG texture
    C3D_TexInit(&screen_tex, SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT, GPU_RGBA8);    
    screenPixels = (Pixel*)linearAlloc(SCREEN_TEXTURE_WIDTH * SCREEN_TEXTURE_HEIGHT * sizeof(Pixel));
    InitialiseBitmap(&screen, SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT, BytesPerRowForWidth(SCREEN_TEXTURE_WIDTH), screenPixels);
    
    // FG texture
    C3D_TexInit(&logo_tex, SCREEN_TEXTURE_HEIGHT, SCREEN_TEXTURE_WIDTH, GPU_RGBA8);
    C3D_TexUpload(&logo_tex, logo_bin);
    C3D_TexSetFilter(&logo_tex, GPU_LINEAR, GPU_NEAREST);
    
    // Create the VBO
    tunnelVBO = (vertex*)linearAlloc(sizeof(vertex) * MAX_VERTS);
    
    C3D_BufInfo* bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, (void*)tunnelVBO, sizeof(vertex), 3, 0x210);
    
    C3D_LightEnvInit(&lightEnv);
    C3D_LightEnvBind(&lightEnv);
    C3D_LightEnvMaterial(&lightEnv, &lightMaterial);

    LightLut_Phong(&lutPhong, 20.0f);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_D0, GPU_LUTINPUT_NH, false, &lutPhong);

    C3D_LightInit(&light, &lightEnv);
}

static void effectTunnelDraw(float iod, float time) {
    C3D_BindProgram(&program);
    
    // Set GPU state
    // C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
    
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
    Mtx_PerspStereoTilt(&projection, 65.0f*M_PI/180.0f, 400.0f/240.0f, 0.2f, 30.0f, iod, 2.0f, false);
    
    // Probably 3D here
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);
    
    // Compute new modelview
    C3D_Mtx modelview;
    Mtx_Identity(&modelview);
    Mtx_Translate(&modelview, 0.0, 0.0, -1.0, true);
    Mtx_RotateZ(&modelview, time * 0.0005, true);
    
    // Send matrices
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocProjection, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocModelview,  &modelview);

    // Bind a texture
    C3D_TexSetFilter(&screen_tex, GPU_LINEAR, GPU_LINEAR);
    C3D_TexBind(0, &logo_tex);
    
    // Stream some new verts
    int vertCount = 0;
    for(int i = 0; i < 10; i++) {
        float xs = sin(i * 1031.0);
        float ys = cos(i * 1031.0);
        vertCount += buildQuad(
            &tunnelVBO[vertCount], 
            vec3(-0.5, -0.5, -i * 0.4),
            vec3( 0.5, -0.5, -i * 0.4),
            vec3( 0.5,  0.5, -i * 0.4),
            vec3(-0.5,  0.5, -i * 0.4),
            vec2(0, 0),
            vec2(1, 0),
            vec2(1, 1),
            vec2(0, 1)
        );
    }
    
    // Draw the VBO
    C3D_DrawArrays(GPU_TRIANGLES, 0, vertCount);
}

void effectTunnelRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float row) {
    float bg_time = sync_get_val(sync_bg_pos, row);
    float scroller_time = sync_get_val(sync_scroll_pos, row);
    
    float xshift = cos(bg_time * 0.0003) * 0.1;
    float yshift = sin(bg_time * 0.0001) * 0.1;
    
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
    
    // Scroller draw
    float sshift = -scroller_time * 0.4;
    FillBitmap(&scroller, RGBAf(1.0, 1.0, 1.0, 0.1));
    DrawSimpleString(&scroller, &OL16Font, sshift, 7 * 16, RGBAf(0.3, 0.6 + 3 * 0.1, 1.0 - 3 * 0.3, 1.0), SCROLLERTEXT);
    
    GSPGPU_FlushDataCache(scrollPixels, 512 * 256 * sizeof(Pixel));
    GX_DisplayTransfer((u32*)scrollPixels, GX_BUFFER_DIM(512, 256), (u32*)scroll_tex.data, GX_BUFFER_DIM(512, 256), TEXTURE_TRANSFER_FLAGS);
    gspWaitForPPF();
    
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    
    // Left eye
    C3D_FrameDrawOn(targetLeft);
    C3D_RenderTargetClear(targetLeft, C3D_CLEAR_ALL, 0x68B0D8FF, 0);
    
    // Background
    fullscreenQuad(screen_tex, -iod, 1.0 / 10.0);
    
    // Actual scene (empty in this, but you could!)
    effectTunnelDraw(-iod, bg_time);
    
    // Scroller as an overlay
    fullscreenQuad(scroll_tex, -iod, 1.0 / 15.0);
    
    // Overlay
    fullscreenQuad(logo_tex, 0.0, 0.0);
  
    fade();
    
    if(iod > 0.0) {
        // Right eye
        C3D_FrameDrawOn(targetRight);
        C3D_RenderTargetClear(targetRight, C3D_CLEAR_ALL, 0x68B0D8FF, 0);
        
        // Background
        fullscreenQuad(screen_tex, iod, 1.0 / 10.0);
        
        // Actual scene
        effectTunnelDraw(iod, bg_time);
    
        // Scroller as an overlay
        fullscreenQuad(scroll_tex, iod, 1.0 / 15.0);
        
        // Overlay
        fullscreenQuad(logo_tex, 0.0, 0.0);
        
        fade();
    }
    
    C3D_FrameEnd(0);
}

void effectTunnelExit() {
    // Free the texture
    C3D_TexDelete(&scroll_tex);
    C3D_TexDelete(&screen_tex);
    C3D_TexDelete(&logo_tex);
    
    // Free vertices
    linearFree(tunnelVBO);
    
    // Free pixel data
    linearFree(scrollPixels);
    linearFree(screenPixels);
    
    // Free the shader program
    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);
}
