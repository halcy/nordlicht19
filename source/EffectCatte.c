// Nordlicht demoparty
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "Tools.h"
#include <vshader_shbin.h>
#include "cat_fg_bin.h"
#include "cat_bg_bin.h"
#include "catface_bin.h"
#include "Catte.h"
#include "Perlin.h"

static DVLB_s* vshader_dvlb;
static shaderProgram_s program;

static int uLoc_projection, uLoc_modelview;
static C3D_Mtx projection;
static C3D_Mtx modelview;

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

C3D_RenderTarget* target_temp;
C3D_Tex temp_tex;

C3D_RenderTarget* target_temp_r;
C3D_Tex temp_tex_r;

#define MAX_VERTS 20000

#include "Font.h"
#include "MonoFont.h"

extern Font OL16Font; 

static Pixel* scroll_pixels;
static Bitmap scroller;

const struct sync_track* sync_rot;
const struct sync_track* sync_glitch;
const struct sync_track* sync_glitch_t;

static const C3D_Material lightMaterial = {
    { 0.1f, 0.1f, 0.1f }, //ambient
    { 0.2f, 0.8f, 0.4f }, //diffuse
    { 0.8f, 0.8f, 0.9f }, //specular0
    { 0.0f, 0.0f, 0.0f }, //specular1
    { 0.0f, 0.1f, 0.0f }, //emission
};

// :3
void effectCatteInit() {
    // Get rocket track
    sync_rot = sync_get_track(rocket, "cat.rot");
    sync_glitch = sync_get_track(rocket, "cat.glitch");
    sync_glitch_t = sync_get_track(rocket, "cat.glitch_t");
    
    // Load default shader
    vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
    shaderProgramInit(&program);
    shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);
    
    // BG texture
    C3D_TexInit(&screen_tex, SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT, GPU_RGBA8);    
    C3D_TexUpload(&screen_tex, cat_bg_bin);
    C3D_TexSetFilter(&screen_tex, GPU_NEAREST, GPU_NEAREST);
    
    // FG texture
    C3D_TexInit(&logo_tex, SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT, GPU_RGBA8);
    C3D_TexUpload(&logo_tex, cat_fg_bin);
    C3D_TexSetFilter(&logo_tex, GPU_NEAREST, GPU_NEAREST);
    
    // :3
    C3D_TexInit(&cat_tex, 64, 64, GPU_RGBA8);
    C3D_TexUpload(&cat_tex, catface_bin);
    C3D_TexSetFilter(&cat_tex, GPU_NEAREST, GPU_NEAREST);
    C3D_TexSetWrap(&cat_tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
    
        
    C3D_TexInit(&temp_tex, 256, 512, GPU_RGBA8);
    target_temp = C3D_RenderTargetCreateFromTex(&temp_tex, GPU_TEXFACE_2D, 0, GPU_RB_DEPTH24_STENCIL8);
    
    C3D_TexInit(&temp_tex_r, 256, 512, GPU_RGBA8);
    target_temp_r = C3D_RenderTargetCreateFromTex(&temp_tex_r, GPU_TEXFACE_2D, 0, GPU_RB_DEPTH24_STENCIL8);
    
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

static void effectCatteDraw(float iod, float row) {
    float cat_rot = sync_get_val(sync_rot, row);
    
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
    
    // Update the uniforms
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection);
    
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
    
    // Depth test on
    C3D_DepthTest(true, GPU_GREATER, GPU_WRITE_ALL);
        
    // Culling too
    C3D_CullFace(GPU_CULL_BACK_CCW);
    
    // Texenv glowy
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_RGB, GPU_FRAGMENT_PRIMARY_COLOR, GPU_FRAGMENT_SECONDARY_COLOR, 0);
    C3D_TexEnvOp(env, C3D_RGB, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_RGB, GPU_ADD);

    C3D_TexEnv* env2 = C3D_GetTexEnv(1);
    C3D_TexEnvSrc(env2, C3D_RGB, GPU_TEXTURE0, GPU_PREVIOUS, 0);
    C3D_TexEnvOp(env2, C3D_RGB, 0, 0, 0);
    C3D_TexEnvFunc(env2, C3D_RGB, GPU_ADD);
    
    // Bind a texture
    C3D_TexBind(0, &cat_tex);
    
    // Calculate the modelview matrix (catte)
    Mtx_Identity(&modelview);
    Mtx_Translate(&modelview, -0.25, 1.0, -6.0, true);
    Mtx_RotateZ(&modelview, 0.2, true);
    Mtx_RotateY(&modelview, cat_rot * 0.003, true);
    Mtx_Scale(&modelview, 2.0, 2.0, 2.0);
    
    // Update the uniforms
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelview,  &modelview);
    
    // Draw the VBO
    C3D_DrawArrays(GPU_TRIANGLES, 0, numFacesCatte * 3);
    
    // Turn lighting off again
    C3D_LightEnvBind(0);
}

void effectCatteRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float row) {
    float glitch = sync_get_val(sync_glitch, row);
    float glitch_t = sync_get_val(sync_glitch_t, row);
        
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    
    // Left eye
    C3D_FrameBufClear(&target_temp->frameBuf, C3D_CLEAR_ALL, 0, 0);
    C3D_FrameDrawOn(target_temp);
    
    // Background
    fullscreenQuad(screen_tex, -iod, 1.0 / 10.0);
    
    // Actual scene (empty in this, but you could!)
    effectCatteDraw(-iod, row);
    
    // Render to actual eye
    C3D_FrameDrawOn(targetLeft);
    fullscreenQuadGlitch(temp_tex, 40, glitch_t, glitch);
    
    // Overlay
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE, GPU_SRC_ALPHA, GPU_ONE);
    fullscreenQuad(logo_tex, 0.0, 0.0);
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
    
    fade();
    
    if(iod > 0.0) {
        // Right eye
        C3D_FrameBufClear(&target_temp_r->frameBuf, C3D_CLEAR_ALL, 0, 0);
        C3D_FrameDrawOn(target_temp_r);
    
        // Background
        fullscreenQuad(screen_tex, iod, 1.0 / 10.0);
        
        // Actual scene
        effectCatteDraw(iod, row);
        
        // Render to actual eye
        C3D_FrameDrawOn(targetRight);
        fullscreenQuadGlitch(temp_tex_r, 40, glitch_t, glitch);
        
        // Overlay
        C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE, GPU_SRC_ALPHA, GPU_ONE);
        fullscreenQuad(logo_tex, 0.0, 0.0);
        C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
        
        fade();
    }
    
    C3D_FrameEnd(0);
}

void effectCatteExit() {
    // Free vertices
    linearFree(vbo_verts);
    
    // Free the textures
    C3D_TexDelete(&screen_tex);
    C3D_TexDelete(&logo_tex);
    C3D_TexDelete(&cat_tex);

    // Remove rendertarget (Crashes - whoops? maybe fine to delete only tex)
//     C3D_RenderTargetDelete(target_temp);
    C3D_TexDelete(&temp_tex);
    C3D_TexDelete(&temp_tex_r);
    
    // Free the shader program
    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);
}
