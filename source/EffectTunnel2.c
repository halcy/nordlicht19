// Nordlicht demoparty
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "Tools.h"
#include <vshader_shbin.h>
#include "tunnel2_bin.h"
#include "cube_rocketship_bin.h"
#include "tunnel2_logo_bin.h"
#include "gridbg_bin.h"
#include "Perlin.h"

#define TUNNEL2_MAX_VERTS 20000

static DVLB_s* vshader_dvlb;
static shaderProgram_s program;

static int uLoc_projection, uLoc_modelView;
static C3D_Mtx projection;

static C3D_Tex sphere_tex;
static C3D_Tex logo_tex;
static C3D_Tex bg_tex;

int32_t vert_count;
int32_t vert_count_2;
static vertex* vboVerts;

#define SEGMENTS 15


const struct sync_track* sync_disp_fact;
const struct sync_track* sync_pos;
const struct sync_track* sync_rot;

// Ohne tunnel geht eben nicht
void effectTunnel2Init() {
    // Load default shader
    vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
    shaderProgramInit(&program);
    shaderProgramSetVsh(&program, &vshader_dvlb->DVLE[0]);

    C3D_TexInit(&sphere_tex, 64, 64, GPU_RGBA8);
    C3D_TexInit(&bg_tex, SCREEN_TEXTURE_HEIGHT, SCREEN_TEXTURE_WIDTH, GPU_RGBA8);
    C3D_TexInit(&logo_tex, SCREEN_TEXTURE_HEIGHT, SCREEN_TEXTURE_WIDTH, GPU_RGBA8);
    
    vboVerts = (vertex*)linearAlloc(sizeof(vertex) * TUNNEL2_MAX_VERTS);
    
    // Load the texture and bind it to the first texture unit
    C3D_TexUpload(&sphere_tex, cube_rocketship_bin);
    C3D_TexSetFilter(&sphere_tex, GPU_LINEAR, GPU_NEAREST);
    
    C3D_TexUpload(&logo_tex, tunnel2_logo_bin);
    C3D_TexSetFilter(&logo_tex, GPU_LINEAR, GPU_NEAREST);
    
    C3D_TexUpload(&bg_tex, gridbg_bin);
    C3D_TexSetFilter(&bg_tex, GPU_LINEAR, GPU_NEAREST);
    
    sync_pos = sync_get_track(rocket, "tunnel2.pos");
    sync_rot = sync_get_track(rocket, "tunnel2.rot");
    sync_disp_fact = sync_get_track(rocket, "tunnel2.dfact");
}

void effectTunnel2Update(float row) {
    int depth = 90;
    float zscale = 0.3;
    float rscale = 1.9;
    float rscale_inner = 1.7;
    float rscale_innerest = 0.3;
    float display_factor = sync_get_val(sync_disp_fact, row); // 0.03
    
    vert_count = 0;
    vert_count_2 = 10000;
    
    vec3_t ringPos[SEGMENTS];
    vec3_t ringPosPrev[SEGMENTS];
    
    vec3_t ringPosInner[SEGMENTS];
    vec3_t ringPosPrevInner[SEGMENTS];
    
    vec3_t ringPosInnerest[SEGMENTS];
    vec3_t ringPosPrevInnerest[SEGMENTS];
    
    float distance = -sync_get_val(sync_pos, row) * 0.03;
    float offset = fmod(distance, 1.0);
    float rot_off = sync_get_val(sync_rot, row);
    for(int z = depth - 1; z >= 0; z--) {
        float zo = z + offset;
        float xo = sin(zo * 0.2) * (zo / 10.0) * cos(rot_off * 0.01);
        float yo = cos(zo * 0.2) * (zo / 10.0) * sin(rot_off * 0.01);
        
        for(int s = 0; s < SEGMENTS; s++) {
            float rn = 0.9;
            float sf = (((float)s) / (float)SEGMENTS) * 2.0 * 3.1415;
            float ho = zo * 0.2 - 1.0;
            ringPos[s] = vec3(
                (sin(sf + rot_off * 0.01) * rscale + xo) * rn, 
                (cos(sf + rot_off * 0.01) * rscale + yo) * rn + ho, 
                -zo * zscale
            );
            ringPosInner[s] = vec3(
                (sin(sf + rot_off * 0.03) * rscale_inner + xo) * rn, 
                (cos(sf + rot_off * 0.03) * rscale_inner + yo) * rn + ho, 
                -zo * zscale
            );
            ringPosInnerest[s] = vec3(
                (sin(sf) * rscale_innerest + xo) * rn, 
                (cos(sf) * rscale_innerest + yo) * rn + ho, 
                -zo * zscale
            );
        }
        
        if(z != depth - 1) {
            for(int s = 0; s < SEGMENTS; s++) {
                int sn = (s + 1) % SEGMENTS;
                float tv = -row;
                float nv = noise_at(z - distance + offset, s, 0.5);
                float nv2 = noise_at((z - distance + offset + tv) * 0.2, 0.1, 0.1);
                float shift = 0.0;
                if(nv2 > 0.0) {
                    shift = 0.5;
                }
                
                if(nv > 0.15 - display_factor) {
                    vert_count += buildQuad(&(vboVerts[vert_count]), ringPosPrev[s], ringPos[s], ringPos[sn], ringPosPrev[sn], 
                                        vec2(0.5, 0.5), vec2(1, 0.5), vec2(1, 1), vec2(0.5, 1));
                }
                
                if(nv < -0.15 + display_factor) {
                    vert_count += buildQuad(&(vboVerts[vert_count]), ringPosPrevInner[s], ringPosInner[s], ringPosInner[sn], ringPosPrevInner[sn], 
                                        vec2(0.0, 0.5), vec2(0.5, 0.5), vec2(0.5, 1.0), vec2(0.0, 1.0));
                }
                
                vert_count_2 += buildQuad(&(vboVerts[vert_count_2]), ringPosPrevInnerest[s], ringPosInnerest[s], ringPosInnerest[sn], ringPosPrevInnerest[sn], 
                                    vec2(0.0 + shift, 0.0), vec2(0.5 + shift, 0.0), vec2(0.5 + shift, 0.5), vec2(0.0 + shift, 0.5));
            }
        }
        
        for(int s = 0; s < SEGMENTS; s++) {
            ringPosPrev[s] = ringPos[s];
            ringPosPrevInner[s] = ringPosInner[s];
            ringPosPrevInnerest[s] = ringPosInnerest[s];
        }
    }
}

// Draw 
void effectTunnel2Draw(float iod) {
    C3D_BindProgram(&program);

    // Get the location of the uniforms
    uLoc_projection = shaderInstanceGetUniformLocation(program.vertexShader, "projection");
    uLoc_modelView = shaderInstanceGetUniformLocation(program.vertexShader, "modelView");

    // Configure attributes for use with the vertex shader
    C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0=position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1=texcoord
    AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 3); // v2=normal

    // Compute the projection matrix
    Mtx_PerspStereoTilt(&projection, 60.0f*M_PI/180.0f, 400.0f/240.0f, 0.01f, 1000.0f, iod, 2.0f, false);

    // Bind texture to the first texture unit
    C3D_TexBind(0, &sphere_tex);
    
    // Calculate the modelView matrix
    C3D_Mtx modelView;
    Mtx_Identity(&modelView);
    Mtx_Translate(&modelView, 0.0, -0.2, 0.0, false);
    
    // Update the uniforms
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_projection, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLoc_modelView,  &modelView);

    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, 0, 0);
    C3D_TexEnvOp(env, C3D_Both, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
    
    C3D_LightEnvBind(0);
    
    // No cull, but depth
    C3D_CullFace(GPU_CULL_NONE); 
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
    
    // Draw the VBO
    C3D_TexBind(0, &sphere_tex);    
    C3D_BufInfo* bufInfo = C3D_GetBufInfo();
    
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, vboVerts, sizeof(vertex), 3, 0x210);
    
    C3D_DrawArrays(GPU_TRIANGLES, 0, vert_count);

    env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, 0, 0);
    C3D_TexEnvOp(env, C3D_Both, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
    
    C3D_TexEnv* env2 = C3D_GetTexEnv(1);
    C3D_TexEnvSrc(env2, C3D_Both, GPU_TEXTURE0, GPU_PREVIOUS, 0);
    C3D_TexEnvOp(env2, C3D_Both, 0, 0, 0);
    C3D_TexEnvFunc(env2, C3D_Both, GPU_ADD);
    
    C3D_DrawArrays(GPU_TRIANGLES, 10000, vert_count_2 - 10000);
    
    // Reset
    C3D_CullFace(GPU_CULL_NONE);
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
    C3D_LightEnvBind(0);
}

void effectTunnel2Render(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float row) {
    effectTunnel2Update(row);
    
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    
    // Left eye
    C3D_FrameDrawOn(targetLeft);
    
    // Background
    fullscreenQuad(bg_tex, -iod, 1.0 / 10.0);
    
    // Actual scene
    effectTunnel2Draw(-iod);
    
    // Overlay
    fullscreenQuad(logo_tex, 0.0, 0.0);
    
    fade();
    
    if(iod > 0.0) {
        // Right eye
        C3D_FrameDrawOn(targetRight);
    
        // Background
        fullscreenQuad(bg_tex, iod, 1.0 / 10.0);
        
        // Actual scene
        effectTunnel2Draw(iod);
        
        // Overlay
        fullscreenQuad(logo_tex, 0.0, 0.0);
        
        fade();
    }
    
    C3D_FrameEnd(0);
}

void effectTunnel2Exit() {
    // Free the texture
    C3D_TexDelete(&sphere_tex);
    C3D_TexDelete(&bg_tex);
    C3D_TexDelete(&logo_tex);
    
    // Free the VBOs
    linearFree(vboVerts);
    
    // Free the shader program
    shaderProgramFree(&program);
    DVLB_Free(vshader_dvlb);
}
