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
#include "tunnel_glow_bin.h"
#include "tunnel1_bg_bin.h"
#include "Perlin.h"

static DVLB_s* vshader_dvlb;
static shaderProgram_s program;

static int uLocProjection;
static int uLocModelview;
static C3D_Mtx projection;

static C3D_Tex bg_tex;
static C3D_Tex logo_tex;
static C3D_Tex tunnel_glow_tex;

static C3D_LightEnv lightEnv;
static C3D_Light light;
static C3D_LightLut lutPhong;

static C3D_FogLut fog_Lut;

#include "Font.h"
#include "MonoFont.h"

extern Font OL16Font; 

#define SCROLLERTEXT "welcome to our small release for nordlicht 2018. Started way too late again, as usual, but here we are! The code and graphics this time are by halcy, and the music is by soto.                     release 3ds prods, you cowards!"

static Pixel* scrollPixels;
static Bitmap scroller;
static C3D_Tex scroll_tex;

const struct sync_track* sync_scroll_pos;

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
    sync_scroll_pos = sync_get_track(rocket, "tunnel.scroller");
    
    // Load default shader
    vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
    shaderProgramInit(&program);
    shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
    
    // Scroller texture
    C3D_TexInit(&scroll_tex, 512, 512, GPU_RGBA8);
    scrollPixels = (Pixel*)linearAlloc(512 * 512 * sizeof(Pixel));
    InitialiseBitmap(&scroller, 512, 512, BytesPerRowForWidth(512), scrollPixels);
    
    // BG texture
    C3D_TexInit(&bg_tex, SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT, GPU_RGBA8);    
    C3D_TexUpload(&bg_tex, tunnel1_bg_bin);
    C3D_TexSetFilter(&bg_tex, GPU_LINEAR, GPU_NEAREST);
    
    // FG texture
    C3D_TexInit(&logo_tex, SCREEN_TEXTURE_HEIGHT, SCREEN_TEXTURE_WIDTH, GPU_RGBA8);
    C3D_TexUpload(&logo_tex, logo_bin);
    C3D_TexSetFilter(&logo_tex, GPU_LINEAR, GPU_NEAREST);
    
    // Tunnel texture
    C3D_TexInit(&tunnel_glow_tex, SCREEN_TEXTURE_HEIGHT, SCREEN_TEXTURE_WIDTH, GPU_RGBA8);
    C3D_TexUpload(&tunnel_glow_tex, tunnel_glow_bin);
    C3D_TexSetFilter(&tunnel_glow_tex, GPU_LINEAR, GPU_NEAREST);
    C3D_TexSetFilterMipmap(&tunnel_glow_tex, GPU_LINEAR);
    C3D_TexSetWrap(&tunnel_glow_tex, GPU_REPEAT, GPU_REPEAT);
    
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
    C3D_TexBind(0, &tunnel_glow_tex);
    
    // Set up texenv
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);
    
    // Set up fog
    FogLut_Exp(&fog_Lut, 0.05f, 1.5f, 0.2f, 500.0f);
    C3D_FogGasMode(GPU_FOG, GPU_PLAIN_DENSITY, false);
    C3D_FogColor(0x00000000);
    C3D_FogLutBind(&fog_Lut);
    
    // Compute new modelview
    float zoff = time * 0.01;
    C3D_Mtx modelview;
    Mtx_Identity(&modelview);
    Mtx_Translate(&modelview, 0.0, 0.0, zoff - 5.0, true);
    Mtx_RotateZ(&modelview, time * 0.00005, true);
    
    // Send matrices
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocProjection, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocModelview,  &modelview);

    C3D_CullFace(GPU_CULL_FRONT_CCW);
    
    // Stream some new verts
    int vertCount = 0;
    float beam_step = 4.0;
    float beam_rad = 1.0;;
    float beam_len = 100.0;
    float beam_offset = 5.0;
    
    for(int i = 25 + (int)zoff / beam_step; i >= (int)zoff / beam_step; i--) {
        srand(i);
        for(int j = 0; j < 100; j++) {
            rand();
        }
        float seed = (float)rand() / 1000000.0;
        float cx = sin(seed);
        float cy = cos(seed);
        float ocx = cy;
        float ocy = -cx;
        float rcx = cx;
        float rcy = cy;
        
        cx *= beam_offset;
        cy *= beam_offset;
        ocx *= beam_len;
        ocy *= beam_len;
        rcx *= beam_rad;
        rcy *= beam_rad;
        
        float xa = cx  - ocx + rcx;
        float ya = cy  - ocy + rcy;
        
        float xb = cx  + ocx + rcx;
        float yb = cy  + ocy + rcy;
        
        float xc = cx  + ocx - rcx;
        float yc = cy  + ocy - rcy;
        
        float xd = cx  - ocx - rcx;
        float yd = cy  - ocy - rcy;
        
        if(i % 2 < 1) { 
            vertCount += buildQuad(
                &tunnelVBO[vertCount], 
                vec3(xa, ya, -i * beam_step),
                vec3(xb, yb, -i * beam_step),
                vec3(xc, yc, -i * beam_step),
                vec3(xd, yd, -i * beam_step),
                vec2(1.0, 0.0),
                vec2(0.5, 0.0),
                vec2(0.5, 0.5),
                vec2(1.0, 0.5)
            );
        }
        else {
            vertCount += buildQuad(
                &tunnelVBO[vertCount], 
                vec3(xa, ya, -i * beam_step),
                vec3(xb, yb, -i * beam_step),
                vec3(xc, yc, -i * beam_step),
                vec3(xd, yd, -i * beam_step),
                vec2(1.0, 0.5),
                vec2(0.5, 0.5),
                vec2(0.5, 1.0),
                vec2(1.0, 1.0)
            );
        }
    }

    // Additive blended draw
    C3D_LightEnvBind(0);
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_COLOR);
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE, GPU_SRC_ALPHA, GPU_ONE);
    C3D_DrawArrays(GPU_TRIANGLES, 0, vertCount);
    
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
    C3D_CullFace(GPU_CULL_NONE);
}

void effectTunnelRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float row) {
//     float bg_time = sync_get_val(sync_bg_pos, row);
    float scroller_time = sync_get_val(sync_scroll_pos, row);
    
//     float xshift = cos(bg_time * 0.0003) * 0.1;
//     float yshift = sin(bg_time * 0.0001) * 0.1;
//     
    // Render some 2D stuff
//     FillBitmap(&screen, RGBAf(0.1 * 0.5, 0.15 * 0.5, 0.15 * 0.5, 1.0));
//     for(int x = 0; x < SCREEN_WIDTH; x += 10) {
//         for(int y = 0; y < SCREEN_HEIGHT; y += 10) {
//             float posX = ((float)(x - SCREEN_WIDTH / 2) / (float)SCREEN_WIDTH) + xshift * 0.1;
//             float posY = ((float)(y - SCREEN_HEIGHT / 2) / (float)SCREEN_WIDTH) - yshift * 0.1;
//             
//             float lines = fmod(posX + posY * 0.3 + bg_time * 0.0001 + 10.0, 0.3) > 0.15 ? 0.15 : 0.05;
//             float lines2 = fmod(posX + posY * 0.2 + bg_time * 0.0002 + 10.0, 0.3) > 0.15 ? 0.55 : 0.05;
//             
//             Pixel colorPrimary = RGBAf(lines2, lines2 * 0.9, lines2, 1.0);
//             Pixel colorSecondary = RGBAf(lines, lines, lines, 1.0);
//             
//             DrawFilledRectangle(&screen, x, y, 10, 10, colorSecondary);
//             DrawFilledCircle(&screen, x + 5, y + 5, 3, colorPrimary);
//         }
//     }
    
//     GSPGPU_FlushDataCache(screenPixels, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(Pixel));
//     GX_DisplayTransfer((u32*)screenPixels, GX_BUFFER_DIM(SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT), (u32*)bg_tex.data, GX_BUFFER_DIM(SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT), TEXTURE_TRANSFER_FLAGS);
//     gspWaitForPPF();
//     
    
    // Scroller draw
    float sshift = -scroller_time * 0.1;
    FillBitmap(&scroller, RGBAf(1.0, 1.0, 1.0, 0.1));
    DrawSimpleString(&scroller, &OL16Font, sshift, 11 * 16 + 10, RGBA(206, 237, 244, 255), SCROLLERTEXT);
    
    GSPGPU_FlushDataCache(scrollPixels, 512 * 256 * sizeof(Pixel));
    GX_DisplayTransfer((u32*)scrollPixels, GX_BUFFER_DIM(512, 256), (u32*)scroll_tex.data, GX_BUFFER_DIM(512, 256), TEXTURE_TRANSFER_FLAGS);
    gspWaitForPPF();
    
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    
    // Left eye
    C3D_FrameDrawOn(targetLeft);
//     C3D_RenderTargetClear(targetLeft, C3D_CLEAR_ALL, 0x68B0D8FF, 0);
    
    // Background
    C3D_DepthTest(false, GPU_GEQUAL, GPU_WRITE_COLOR);
    fullscreenQuad(bg_tex, -iod, 1.0 / 10.0);
    
    // Actual scene (empty in this, but you could!)
    effectTunnelDraw(-iod, row * 100.0);
    
    // Overlay
    fullscreenQuad(logo_tex, 0.0, 0.0);
  
    // Scroller as an overlay
    fullscreenQuad(scroll_tex, -iod, 1.0 / 15.0);
    
    fade();
    
    if(iod > 0.0) {
        // Right eye
        C3D_FrameDrawOn(targetRight);
//         C3D_RenderTargetClear(targetRight, C3D_CLEAR_ALL, 0x68B0D8FF, 0);
        
        // Background
        C3D_DepthTest(false, GPU_GEQUAL, GPU_WRITE_COLOR);
        fullscreenQuad(bg_tex, iod, 1.0 / 10.0);
        
        // Actual scene
        effectTunnelDraw(iod, row * 100.0);
        
        // Overlay
        fullscreenQuad(logo_tex, 0.0, 0.0);
        
        // Scroller as an overlay
        fullscreenQuad(scroll_tex, iod, 1.0 / 15.0);
        
        fade();
    }
    
    C3D_FrameEnd(0);
}

void effectTunnelExit() {
    // Free the texture
    C3D_TexDelete(&scroll_tex);
    C3D_TexDelete(&bg_tex);
    C3D_TexDelete(&logo_tex);
    
    // Fog off
    C3D_FogGasMode(GPU_NO_FOG, GPU_DEPTH_DENSITY, false);
    
    // Free vertices
    linearFree(tunnelVBO);
    
    // Free pixel data
    linearFree(scrollPixels);
//     linearFree(screenPixels);
    
    // Free the shader program
    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);
}
