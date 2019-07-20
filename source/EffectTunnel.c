// Nordlicht demoparty
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "Tools.h"
#include "Perlin.h"

#include "ModelTunnel.h"

static C3D_AttrInfo* attrInfo;

static int uLocProjection;
static int uLocModelview;
static C3D_Mtx projection;

static C3D_LightEnv lightEnv;
static C3D_Light light;
static C3D_LightLut lutPhong;
static C3D_LightLut lutShittyFresnel;

static C3D_Tex tunnelTex;
static C3D_Tex tunnelTexGlow;
static C3D_Tex skaterboi;

const struct sync_track* sync_rot;
const struct sync_track* sync_z;
const struct sync_track* sync_laser_seed;
const struct sync_track* sync_laser_start;
const struct sync_track* sync_laser_end;
const struct sync_track* sync_finallogo;
const struct sync_track* sync_light_r;
const struct sync_track* sync_light_gb;

static const C3D_Material lightMaterial = {
    { 0.1f, 0.1f, 0.1f }, //ambient
    { 1.0,  1.0,  1.0 }, //diffuse
    { 0.0f, 0.0f, 0.0f }, //specular0
    { 0.0f, 0.0f, 0.0f }, //specular1
    { 0.0f, 0.0f, 0.0f }, //emission
};

// Boooones
static int uLocBone[21];

static vertex_rigged* vbo;
int32_t vertCount;
#define MAX_VERTS 35000

// Powertunnel
void effectTunnelInit() {
    // Get rocket tracks
    sync_rot = sync_get_track(rocket, "tunnel.rot");
    sync_z = sync_get_track(rocket, "tunnel.z");
    sync_laser_seed = sync_get_track(rocket, "tunnel.lseed");
    sync_laser_start = sync_get_track(rocket, "tunnel.lstart");
    sync_laser_end = sync_get_track(rocket, "tunnel.lend");
    sync_finallogo = sync_get_track(rocket, "tunnel.logo");
    sync_light_r = sync_get_track(rocket, "tunnel.r");
    sync_light_gb = sync_get_track(rocket, "tunnel.gb");
    
    // Create the VBO
    vbo = (vertex_rigged*)linearAlloc(sizeof(vertex_rigged) * MAX_VERTS);
    
    // Load objects
    memcpy(&vbo[0], tunnelVerts, tunnelNumVerts * sizeof(vertex_rigged));
    
    C3D_BufInfo* bufInfo = C3D_GetBufInfo();
    BufInfo_Init(bufInfo);
    BufInfo_Add(bufInfo, (void*)vbo, sizeof(vertex_rigged), 5, 0x43210);
    
    char boneName[255];
    for(int i = 0; i < 21; i++) {
        sprintf(boneName, "bone%02d", i);
        uLocBone[i] = shaderInstanceGetUniformLocation(shaderProgramBones.vertexShader, boneName);
    }
    
    loadTex3DS(&tunnelTex, NULL, "romfs:/tex_tunnel.bin");
    C3D_TexSetFilter(&tunnelTex, GPU_LINEAR, GPU_LINEAR);
    C3D_TexSetWrap(&tunnelTex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);
    
    loadTex3DS(&tunnelTexGlow, NULL, "romfs:/tex_tunnel_glow.bin");
    C3D_TexSetFilter(&tunnelTexGlow, GPU_LINEAR, GPU_LINEAR);
    C3D_TexSetWrap(&tunnelTexGlow, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);        
    
    loadTex3DS(&skaterboi, NULL, "romfs:/skaterboi.bin");
    C3D_TexSetFilter(&tunnelTexGlow, GPU_NEAREST, GPU_NEAREST);
    C3D_TexSetWrap(&tunnelTexGlow, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);        
}

static void effectTunnelDraw(float iod, float row) {
    C3D_BindProgram(&shaderProgramBones);
    C3D_CullFace(GPU_CULL_BACK_CCW);
    
    // Get the location of the uniforms
    uLocProjection = shaderInstanceGetUniformLocation(shaderProgramBones.vertexShader, "projection");
    uLocModelview = shaderInstanceGetUniformLocation(shaderProgramBones.vertexShader, "modelView");

    // Compute the projection matrix
    Mtx_PerspStereoTilt(&projection, 70.0f*M_PI/180.0f, 400.0f/240.0f, 0.2f, 6000.0f, iod, 2.0f, false);
    
     // Compute new modelview
    float tunnel_z = sync_get_val(sync_z, row);
    float tunnel_rot = sync_get_val(sync_rot, row);
    
    float laser_seed = sync_get_val(sync_laser_seed, row);
    float laser_start = sync_get_val(sync_laser_start, row);
    float laser_end = sync_get_val(sync_laser_end, row);
    
    float r_val = sync_get_val(sync_light_r, row);
    float gb_val = sync_get_val(sync_light_gb, row);
    
    // Send matrices
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocProjection, &projection);
    
    // Normal lightenv
    C3D_LightEnvInit(&lightEnv);
    C3D_LightEnvBind(&lightEnv);
    
    LightLut_Phong(&lutPhong, 100.0);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_D0, GPU_LUTINPUT_LN, false, &lutPhong);
    
    LightLut_FromFunc(&lutShittyFresnel, badFresnel, 1.9, false);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_FR, GPU_LUTINPUT_NV, false, &lutShittyFresnel);
    C3D_LightEnvFresnel(&lightEnv, GPU_PRI_SEC_ALPHA_FRESNEL);
    
    C3D_FVec lightVec = FVec4_New(0.0, 0.0, -4.0, 1.0);
    C3D_LightInit(&light, &lightEnv);
    C3D_LightColor(&light, r_val, gb_val, gb_val);
    C3D_LightPosition(&light, &lightVec);
    
    // Set up for drawing the tunnel
    C3D_TexEnv* env = C3D_GetTexEnv(0);
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
    
    // Render tunnel segments
    C3D_Mtx boneMat;   
    for(int seg = 0; seg < 3; seg++) {
        float seg_id = (int)(tunnel_z / 30.0);
        float seg_off = (-seg_id + seg) * 30.0;
        float in_seg_pos = fmod(tunnel_z, 30.0);
        
        srand(seg_id - seg);
        for(int j = 0; j < 100; j++) {
            rand();
        }
        float rot_speed = ((rand() % 100000) / 100000.0) - 0.5;
        float seg_rot = tunnel_rot * rot_speed;
        
        // Tunnel segment    
        C3D_Mtx modelview;
        Mtx_Identity(&modelview);
        Mtx_Translate(&modelview, 0.0, 0.0, tunnel_z + seg_off - 90.0, true);
        Mtx_RotateZ(&modelview, seg_rot, true);
        Mtx_RotateY(&modelview, M_PI, true);
        Mtx_RotateX(&modelview, M_PI * 0.5, true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocModelview,  &modelview);
        
        for(int i = 0; i < 16; i++) {
            boneMat.m[i] = 0.0;
        }
        boneMat.m[0] = 1.0;
        boneMat.m[5] = 1.0;
        boneMat.m[10] = 1.0;
        for(int i = 0; i < 21; i++) {
            C3D_FVUnifMtx3x4(GPU_VERTEX_SHADER, uLocBone[i], &boneMat);
        }
        
        int animPos = (int)(seg * 30.0 + in_seg_pos) - 30;
        animPos = max(0, min(animPos, 59));
        
        for(int i = 0; i < 3; i++) {
            Mtx_Identity(&boneMat);
            for(int j = 0; j < 4 * 3; j++) {
                boneMat.m[j] = tunnelAnim[animPos][i][j];
            }
            
            C3D_FVUnifMtx3x4(GPU_VERTEX_SHADER, uLocBone[i], &boneMat);
        }
        
        C3D_TexBind(0, &tunnelTex);
        C3D_LightEnvMaterial(&lightEnv, &lightMaterial);
        C3D_DrawArrays(GPU_TRIANGLES, 0, tunnelNumVerts);
    }
    
    // Bind a texture
    C3D_TexBind(0, &tunnelTexGlow);
    
    // Set up texenv
    env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_PRIMARY_COLOR, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_MODULATE);

    C3D_CullFace(GPU_CULL_FRONT_CCW);
    
    // Stream some new verts
    int vertCount = 0;
    int vertStart = tunnelNumVerts;
    float beam_step = 3.0;
    float beam_rad = 0.35;;
    float beam_len = 100.0;
    float beam_offset = 2.5;
    
    C3D_LightEnvBind(0);
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_COLOR);
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE, GPU_SRC_ALPHA, GPU_ONE);
    
    int laserId = 0;
    for(int seg = 0; seg < 3; seg++) {
        float seg_id = (int)(tunnel_z / 30.0);
        float seg_off = (-seg_id + seg) * 30.0;
        float in_seg_pos = fmod(tunnel_z, 30.0);
        
        srand(seg_id - seg);
        for(int j = 0; j < 100; j++) {
            rand();
        }
        float rot_speed = ((rand() % 100000) / 100000.0) - 0.5;
        float seg_rot = tunnel_rot * rot_speed;
        
        // Tunnel segment lasers 
        C3D_Mtx modelview;
        Mtx_Identity(&modelview);
        Mtx_Translate(&modelview, 0.0, 0.0, tunnel_z + seg_off - 90.0, true);
        Mtx_RotateZ(&modelview, seg_rot, true);
//         Mtx_RotateY(&modelview, M_PI, true);
//         Mtx_RotateX(&modelview, M_PI * 0.5, true);
        C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocModelview,  &modelview);
        
        int roundVertCount = 0;
        for(int i = 0; i < 10; i++) {
            laserId++;
            
            float seed = (float)rand() / 1000000.0;
            float laser_off = (int)laser_seed;
            srand(seed + laser_off);
            int showLaser = rand() % 2 == 0 ? 0 : 1;
            
            if(laserId - seg_id * 10 < laser_start || laserId - seg_id * 10 > laser_end || (laser_seed > 0 && showLaser == 0)) {
                continue;
            }
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
            
            float bz = i * beam_step;
            
            int newVerts = buildQuadRigged(
                &vbo[tunnelNumVerts + vertCount], 
                vec3(xa, ya, bz),
                vec3(xb, yb, bz),
                vec3(xc, yc, bz),
                vec3(xd, yd, bz),
                vec2(1.0, 0.0),
                vec2(0.5, 0.0),
                vec2(0.5, 0.5),
                vec2(1.0, 0.5),
                3
            );
            vertCount += newVerts;
            roundVertCount += newVerts;
        }
        
        // Additive blended draw
        if(roundVertCount > 0) {
            C3D_DrawArrays(GPU_TRIANGLES, vertStart, roundVertCount);
            vertStart += roundVertCount;
        }
    }
}

void effectTunnelRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float row) {
    // Set up attribute info
    attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0 = position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 4); // v1 = bone indices
    AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 4); // v2 = bone weights
    AttrInfo_AddLoader(attrInfo, 3, GPU_FLOAT, 3); // v3 = normal        
    AttrInfo_AddLoader(attrInfo, 4, GPU_FLOAT, 2); // v4 = texcoords
    
    float logo_val = sync_get_val(sync_finallogo, row);
    
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    
    // Left eye
    C3D_FrameDrawOn(targetLeft);
    C3D_RenderTargetClear(targetLeft, C3D_CLEAR_ALL, 0x68B0D8FF, 0);
    
    // Actual scene
    effectTunnelDraw(-iod, row);

    if(iod > 0.0) {
        // Right eye
        C3D_FrameDrawOn(targetRight);
        C3D_RenderTargetClear(targetRight, C3D_CLEAR_ALL, 0x68B0D8FF, 0); 

        // Actual scene
        effectTunnelDraw(iod, row);
        C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
        C3D_CullFace(GPU_CULL_BACK_CCW);
        fade();
        
        if(logo_val == 1) {
            fullscreenQuadHRNS(skaterboi, 0.0, 0.0);
        }       
    }
    
    C3D_FrameDrawOn(targetLeft);
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
    C3D_CullFace(GPU_CULL_BACK_CCW);
    fade();
    if(logo_val == 1) {
        fullscreenQuadHRNS(skaterboi, 0.0, 0.0);
    } 
    
    C3D_FrameEnd(0);
}

void effectTunnelExit() {
//     gspWaitForP3D();
//     gspWaitForPPF();
//     
    // Free the texture
    C3D_TexDelete(&tunnelTex);
    C3D_TexDelete(&tunnelTexGlow);
    C3D_TexDelete(&skaterboi);
    
    // Free vertices
    linearFree(vbo);
}
