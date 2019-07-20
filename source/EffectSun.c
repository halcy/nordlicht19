// Nordlicht demoparty
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "Tools.h"

#include "MarchingCubes.h"

#define MAX_METABALL_VERTS 70000
#define MAX_STATIC_VERTS 20000
#define GRID_SIZE 20
#define CELL_SIZE ((0.05 * 32) / GRID_SIZE)

#define CELL(x, y, z) ((z) + (y) * (GRID_SIZE) + (x) * (GRID_SIZE * GRID_SIZE))

#define GRID_OFFSET (((float)GRID_SIZE * CELL_SIZE) / 2.0)
#define COORDS(x, y, z) (vec3((x) * CELL_SIZE - GRID_OFFSET, (y) * CELL_SIZE - GRID_OFFSET, (z) * CELL_SIZE - GRID_OFFSET))
#define GRID_INVERSE(a) ((int)((a + GRID_OFFSET) * (1.0 / CELL_SIZE)))

#define CENT_DIFF(f, c, d, t) vec3(f(c.x - d, c.y, c.z, t) - f(c.x + d, c.y, c.z, t), f(c.x, c.y - d, c.z, t) - f(c.x, c.y + d, c.z, t), f(c.x, c.y, c.z - d, t) - f(c.x, c.y, c.z + d, t))

#include "SpaceRoom.h"

// int CELL(int x, int y, int z) {
//     if(x < 0 || x >= GRID_SIZE) {
//         printf("PROBLEM! x y z = %d %d %d\n", x, y, z);
//         waitForA();
//     }
//     if(y < 0 || y >= GRID_SIZE) {
//         printf("PROBLEM! x y z = %d %d %d\n", x, y, z);
//         waitForA();
//     }
//     if(z < 0 || z >= GRID_SIZE) {
//         printf("PROBLEM! x y z = %d %d %d\n", x, y, z);
//         waitForA();
//     }
//     return ((z) + (y) * (GRID_SIZE) + (x) * (GRID_SIZE * GRID_SIZE));
// }

// Sync
const struct sync_track* sync_anim_t;
const struct sync_track* sync_anim_r;
const struct sync_track* sync_anim_w;
const struct sync_track* sync_anim_f;
const struct sync_track* sync_rotate;
const struct sync_track* sync_bright;
const struct sync_track* sync_build;

// Marching cubes related
float* marchingCubesGrid;
vec3_t* marchingCubesNormals;
int metaballVertCount;

typedef struct gridloc {
    int x;
    int y;
    int z;
} gridloc;
gridloc* marchingCubesQueue;
int* marchingCubesMarkGrid;
int marchingCubesMarkNb; 

// Shader / textures
static int uLocProjection;
static int uLocModelview;
static int uLocProjectionSkybox;
static int uLocModelviewSkybox;

static C3D_Tex skybox_tex;
static C3D_TexCube skybox_cube;
int skyboxVertCount;

static C3D_Tex room_tex;

static C3D_AttrInfo* attrInfo;

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
    { 0.3f, 0.3f, 0.3f }, //ambient
    { 0.5f, 0.5f, 0.5f }, //diffuse
    { 0.5f, 0.4f, 0.4f }, //specular0
    { 0.0f, 0.0f, 0.0f }, //specular1
    { 0.0f, 0.0f, 0.0f }, //emission
};

// VBOs
static vertex* metaballVBO;
static C3D_BufInfo* metaballBufInfo;

void effectSunInit() {
    // Get sync tracks
    sync_anim_t = sync_get_track(rocket, "metaroom.anim_t");
    sync_anim_r = sync_get_track(rocket, "metaroom.anim_r");
    sync_anim_w = sync_get_track(rocket, "metaroom.anim_w");
    sync_anim_f = sync_get_track(rocket, "metaroom.field");
    sync_rotate = sync_get_track(rocket, "metaroom.cam_rot");
    sync_bright = sync_get_track(rocket, "metaroom.bright");
    sync_build  = sync_get_track(rocket, "metaroom.build");
//     printf("Begin\n");
//     waitForA(); 
    
    // Set up "normal 3D rendering" shader and get uniform locations
    C3D_BindProgram(&shaderProgram);
    uLocProjection = shaderInstanceGetUniformLocation(shaderProgram.vertexShader, "projection");
    uLocModelview = shaderInstanceGetUniformLocation(shaderProgram.vertexShader, "modelView");
    
    C3D_BindProgram(&shaderProgramSkybox);
    uLocProjectionSkybox = shaderInstanceGetUniformLocation(shaderProgramSkybox.vertexShader, "projection");
    uLocModelviewSkybox = shaderInstanceGetUniformLocation(shaderProgramSkybox.vertexShader, "modelView");
    
    // Allocate VBOs
//     printf("Lin Alloc");
//     waitForA();
    metaballVBO = (vertex*)linearAlloc(sizeof(vertex) * (MAX_METABALL_VERTS + MAX_STATIC_VERTS));
    
    metaballBufInfo = C3D_GetBufInfo();
    BufInfo_Init(metaballBufInfo);
    BufInfo_Add(metaballBufInfo, (void*)metaballVBO, sizeof(vertex), 3, 0x210);
//     printf("A");
    // Allocate grid
//     printf("Alloc 1");
//     waitForA();
    marchingCubesGrid = (float*)malloc(GRID_SIZE * GRID_SIZE * GRID_SIZE * sizeof(float));
//     printf("Alloc 2");
//     waitForA();
    marchingCubesNormals = (vec3_t*)malloc(GRID_SIZE * GRID_SIZE * GRID_SIZE * sizeof(vec3_t));
//     printf("Alloc 3");
//     waitForA();
    marchingCubesMarkGrid = (int*)malloc(GRID_SIZE * GRID_SIZE * GRID_SIZE * sizeof(int));
/*    printf("Alloc 4");
    waitForA();  */  
    marchingCubesQueue = (gridloc*)malloc(GRID_SIZE * GRID_SIZE * GRID_SIZE * sizeof(gridloc));
    marchingCubesMarkNb = 0;
    
//     printf("%x %x %x %x\n", marchingCubesGrid, marchingCubesNormals, marchingCubesMarkGrid, marchingCubesQueue);
//     printf("----- %d %d %d\n", linearSpaceFree(), vramSpaceFree(), mappableSpaceFree());
//     printf("B");
    // Load statics
    loadObject2to1(numFacesSpaceRoom, facesSpaceRoom, verticesSpaceRoom, normalsSpaceRoom, texcoordsSpaceRoom, &metaballVBO[MAX_METABALL_VERTS]);
//     printf("C");
    skyboxVertCount = buildCube(&metaballVBO[MAX_METABALL_VERTS + numFacesSpaceRoom * 3], vec3(0, 0, 0), 100.0, 0.0, 0.0);
//     printf("D");
    // Load texture for the skybox
    loadTex3DS(&skybox_tex, &skybox_cube, "romfs:/stars.bin");
    C3D_TexSetFilter(&skybox_tex, GPU_LINEAR, GPU_LINEAR);
    C3D_TexSetWrap(&skybox_tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE);    
    
//     printf("E");
    // And texture for the room
    loadTex3DS(&room_tex, NULL, "romfs:/tex_room.bin");
    C3D_TexSetFilter(&room_tex, GPU_LINEAR, GPU_LINEAR);
    C3D_TexSetWrap(&room_tex, GPU_CLAMP_TO_EDGE, GPU_CLAMP_TO_EDGE); 
//     printf("X");
}

float field_torus(float xx, float yy, float zz) {
    float rad_large = 0.22;
    float rad_small = 0.1;    
    float aa = sqrt(xx*xx + zz*zz) - rad_large;
    float bb = sqrt(aa * aa + yy * yy);
    float dd = bb - rad_small;
    return dd < 0.0 ? fabs(dd) * 15.0 : 0.0;
}

float field_ball(float xx, float yy, float zz, float t, float r) {
    xx += sin(yy * t + t) * 0.05;
    zz += cos(yy * t + t) * 0.05;
    return sqrt(xx * xx + yy * yy + zz * zz) - r;
}

float field_box(float xx, float yy, float zz, float r) {
    float dd = fmax(fmax(fabs(xx), fabs(yy)), fabs(zz)) - 0.5;
    return dd < 0.0 ? fabs(dd) * 1.5: 0.0;
}

// A single metaball call.
// function:
// f(x,y,z) = 1 / ((x − x0)^2 + (y − y0)^2 + (z − z0)^2)
float metaball(float x, float y, float z, float cx, float cy, float cz) {
    float dx = x - cx;
    float dy = y - cy;
    float dz = z - cz;
    float xs = dx * dx;
    float ys = dy * dy;
    float zs = dz * dz;
    return(1.0 / sqrt(xs + ys + zs));
}

// Trefoil knot based positioning
vec3_t trefoil(float t) {
    return vec3(
        sin(t) + 2.0 * sin(2.0 * t),
        cos(t) - 2.0 * cos(2.0 * t),
        -sin(3.0 * t)
    );
}

#define NUMBALLS 10
#define NUMBOXEN 4
float xa[NUMBALLS];
float ya[NUMBALLS];
float za[NUMBALLS];
float fieldBalls(float xx, float yy, float zz, float t, float r) {
    float val = 0.0;
    for(int i = 0; i < NUMBALLS; i++) {
        val += metaball(xx, yy, zz, xa[i], ya[i], za[i]);
    }

    return(1.0 / (val / NUMBALLS)  - r);
}

float fieldBoxen(float xx, float yy, float zz, float t, float r) {
    float val = 0.0;
    for(int i = 0; i < NUMBOXEN; i++) {
        val += field_box(xx - xa[i], yy - ya[i], zz - za[i], r);
    }
    if(val == 0) {
        return 10.0;
    }
    return(1.0 / (val / NUMBOXEN)  - r * 10.0);
}

int firstframe = 5;
void effectSunUpdate(float row) {
//     printf("UPD");
    // You can update textures here, but afterwards you have to display transfer and wait for PPF interrupt:
    // GSPGPU_FlushDataCache(screenPixels, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(Pixel));
    // GX_DisplayTransfer((u32*)screenPixels, GX_BUFFER_DIM(SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT), (u32*)screen_tex.data, GX_BUFFER_DIM(SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT), TEXTURE_TRANSFER_FLAGS);
    // gspWaitForPPF();
    
    // Or vertices or any other state ofc.
    // How to get a value from a sync track:
    float anim_t = sync_get_val(sync_anim_t, row);
    float anim_r = sync_get_val(sync_anim_r, row);
    int anim_w = min((int)sync_get_val(sync_anim_w, row), GRID_SIZE);
    int field = (int)sync_get_val(sync_anim_f, row);
    int build_max = (int)sync_get_val(sync_build, row);
    
    startPerfCounter(1);
    
    // Update balls
    xa[0] = 0;
    ya[0] = 0;
    za[0] = 0;
    for(int i = 1; i < NUMBALLS; i++) {
        vec3_t pos = trefoil(anim_t + i * (M_PI * 2.0) / NUMBALLS);
        xa[i] = pos.x * 0.15;
        ya[i] = pos.y * 0.15;
        za[i] = pos.z * 0.15;
    }
    
    // Set grid
    
    if(field == 0) {
        for(int x = anim_w; x < GRID_SIZE; x++) {
            for(int y = 0; y < GRID_SIZE; y++) {
                for(int z = 0; z < GRID_SIZE; z++) {
                    vec3_t coords = COORDS(x, y, z);
                    marchingCubesGrid[CELL(x, y, z)] = fieldBalls(coords.x, coords.y, coords.z, anim_t, anim_r);
                }
            }
        }
        for(int x = 0; x < anim_w; x++) {
            for(int y = 0; y < GRID_SIZE; y++) {
                for(int z = 0; z < GRID_SIZE; z++) {
                    vec3_t coords = COORDS(x, y, z);
                    marchingCubesGrid[CELL(x, y, z)] = field_ball(coords.x, coords.y, coords.z, anim_t, anim_r);
                }
            }
        }
    }
    
    if(field == 1) {        
        for(int x = anim_w; x < GRID_SIZE; x++) {
            for(int y = 0; y < GRID_SIZE; y++) {
                for(int z = 0; z < GRID_SIZE; z++) {
                    vec3_t coords = COORDS(x, y, z);
                    marchingCubesGrid[CELL(x, y, z)] = field_ball(coords.x, coords.y, coords.z, anim_t, anim_r);
                }
            }
        }
        
        for(int x = 0; x < anim_w; x++) {
            for(int y = 0; y < GRID_SIZE; y++) {
                for(int z = 0; z < GRID_SIZE; z++) {
                    vec3_t coords = COORDS(x, y, z);
                    marchingCubesGrid[CELL(x, y, z)] = fieldBoxen(coords.x, coords.y, coords.z, anim_t, anim_r);
                }
            }
        }        
    }
    
    stopPerfCounter(1);
    startPerfCounter(2);
    //printf("died BEFORE normals\n");
    // Set normals for grid (TODO excluding last layer - that okay? probably is - just choose fields wisely)
    for(int x = 0; x < GRID_SIZE - 2; x++) {
        for(int y = 0; y < GRID_SIZE - 2; y++) {
            for(int z = 0; z < GRID_SIZE - 2; z++) {
                vec3_t coords = COORDS(x, y, z);
                float cent_val = marchingCubesGrid[CELL(x, y, z)];
                vec3_t normal = vec3(
                    marchingCubesGrid[CELL(x + 1, y, z)] - cent_val,
                    marchingCubesGrid[CELL(x, y + 1, z)] - cent_val,
                    marchingCubesGrid[CELL(x, y, z + 1)] - cent_val
                );
                marchingCubesNormals[CELL(x, y, z)] = vec3norm(normal);
            }
        }
    }
    
    stopPerfCounter(2);
    startPerfCounter(3);
    //printf("died BEFORE initcell\n");
    
    // Set up mesh creation
    metaballVertCount = 0;
    vec3_t corners[8];
    float values[8];
    vec3_t normals[8];
    int queuePos = 0;
    marchingCubesMarkNb += 1;
    int newVertCount = 0;
    int found_cell = 0;
    
    // Find initial cell
    for(int i = 0; i < GRID_SIZE - 1; i++) {
        if(
            marchingCubesGrid[CELL(i, GRID_SIZE / 2, GRID_SIZE / 2)] > 0 &&
            marchingCubesGrid[CELL(i + 1, GRID_SIZE / 2, GRID_SIZE / 2)] < 0
            
        ) {
            marchingCubesQueue[queuePos].x = i;
            marchingCubesQueue[queuePos].y = GRID_SIZE / 2;
            marchingCubesQueue[queuePos].z = GRID_SIZE / 2;
            marchingCubesMarkGrid[CELL(i, GRID_SIZE / 2, GRID_SIZE / 2)] = marchingCubesMarkNb;
            queuePos += 1;
            found_cell = 1;
            break;
        }
    }
    
    if(found_cell == 0 || build_max == 0) {
        stopPerfCounter(3);
        return;
    }
    //printf("died BEFORE marchingcubes\n");
    
    // March on, cubes
    int queuePosProc = -1;
    int totalProc = 0;
    int maxQueueSize = 0;
    while(queuePosProc < queuePos - 1) {
        queuePosProc += 1;

        int32_t x = marchingCubesQueue[queuePosProc].x;
        int32_t y = marchingCubesQueue[queuePosProc].y;
        int32_t z = marchingCubesQueue[queuePosProc].z;
        int32_t xx = x + 1;
        int32_t yy = y + 1;
        int32_t zz = z + 1;
        
        totalProc += 1;
        
        if(totalProc > build_max) {
            break;
        }
        
        vec3_t coords_a = COORDS(x, y, z);
        vec3_t coords_b = COORDS(xx, yy, zz);

        corners[0] = vec3(coords_a.x, coords_a.y, coords_b.z);
        corners[1] = vec3(coords_b.x, coords_a.y, coords_b.z);
        corners[2] = vec3(coords_b.x, coords_a.y, coords_a.z);
        corners[3] = vec3(coords_a.x, coords_a.y, coords_a.z);
        corners[4] = vec3(coords_a.x, coords_b.y, coords_b.z);
        corners[5] = vec3(coords_b.x, coords_b.y, coords_b.z);
        corners[6] = vec3(coords_b.x, coords_b.y, coords_a.z);
        corners[7] = vec3(coords_a.x, coords_b.y, coords_a.z);
        
        values[0] = marchingCubesGrid[CELL(x, y, zz)];
        values[1] = marchingCubesGrid[CELL(xx, y, zz)];
        values[2] = marchingCubesGrid[CELL(xx, y, z)];
        values[3] = marchingCubesGrid[CELL(x, y, z)];
        values[4] = marchingCubesGrid[CELL(x, yy, zz)];
        values[5] = marchingCubesGrid[CELL(xx, yy, zz)];
        values[6] = marchingCubesGrid[CELL(xx, yy, z)];
        values[7] = marchingCubesGrid[CELL(x, yy, z)];
        
        normals[0] = marchingCubesNormals[CELL(x, y, zz)];
        normals[1] = marchingCubesNormals[CELL(xx, y, zz)];
        normals[2] = marchingCubesNormals[CELL(xx, y, z)];
        normals[3] = marchingCubesNormals[CELL(x, y, z)];
        normals[4] = marchingCubesNormals[CELL(x, yy, zz)];
        normals[5] = marchingCubesNormals[CELL(xx, yy, zz)];
        normals[6] = marchingCubesNormals[CELL(xx, yy, z)];
        normals[7] = marchingCubesNormals[CELL(x, yy, z)];
        
        newVertCount = polygonise(corners, values, normals, 0.0, &(metaballVBO[metaballVertCount]));
        metaballVertCount += newVertCount;
        
        if(newVertCount != 0) {
            int xds = max(x - 1, 0);
            int yds = max(y - 1, 0);
            int zds = max(z - 1, 0);
            int xde = min(x + 1, GRID_SIZE - 2);
            int yde = min(y + 1, GRID_SIZE - 2);
            int zde = min(z + 1, GRID_SIZE - 2);
            for(int xd = xds; xd <= xde; xd++) {
                for(int yd = yds; yd <= yde; yd++) {
                    for(int zd = zds; zd <= zde; zd++) {
                        if(marchingCubesMarkGrid[CELL(xd, yd, zd)] != marchingCubesMarkNb) {
                            marchingCubesQueue[queuePos].x = xd;
                            marchingCubesQueue[queuePos].y = yd;
                            marchingCubesQueue[queuePos].z = zd;
                            marchingCubesMarkGrid[CELL(xd, yd, zd)] = marchingCubesMarkNb;
                            queuePos += 1;
                        }
                    }
                }
            }
        }
        maxQueueSize = max(queuePos, maxQueueSize);
    }
    
//     printf("died AFTER marchingcubes (vc %d %d %d %d)\n", metaballVertCount, queuePos, queuePosProc, anim_w);
    stopPerfCounter(3);
}

void effectSunDraw(float iod, float row) {
//     printf("REND");
    // Projection matrix
    C3D_Mtx projection;
    Mtx_PerspStereoTilt(&projection, 65.0f * M_PI / 180.0f, 400.0f / 240.0f, 0.2f, 500.0f, iod, 2.0f, false);
    
    // Compute MV matrix, uniforms to shader
    C3D_Mtx modelview;
    Mtx_Identity(&modelview);
    
    float rotPathVal = sync_get_val(sync_rotate, row) * 0.1;
    Mtx_Translate(&modelview, 0.0, 0.0, -2.8, true);
    Mtx_RotateY(&modelview, rotPathVal, true);
    Mtx_RotateZ(&modelview, sin(rotPathVal * 0.1) * 0.1, true);
    
    //printf("Died before skybox");
    // Skybox shader
    C3D_BindProgram(&shaderProgramSkybox);
    
    // Uniforms to shader
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocProjectionSkybox, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocModelviewSkybox, &modelview);
    
    // Passthrough texenv
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, 0, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
    
    // Lets draw a skybox
    C3D_TexBind(0, &skybox_tex);
    
    C3D_DrawArrays(GPU_TRIANGLES, MAX_METABALL_VERTS + numFacesSpaceRoom * 3, skyboxVertCount);
    
    //printf("Died before second bind");
    // Normal drawing shader
    C3D_BindProgram(&shaderProgram);
    
    // GPU state for normal drawing
    C3D_DepthTest(true, GPU_GEQUAL, GPU_WRITE_ALL);
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);
    
    C3D_LightEnvInit(&lightEnv);
    C3D_LightEnvBind(&lightEnv);
    C3D_LightEnvMaterial(&lightEnv, &lightMaterial);

    LightLut_Phong(&lutPhong, 100.0);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_D0, GPU_LUTINPUT_LN, false, &lutPhong);
    
    LightLut_FromFunc(&lutShittyFresnel, badFresnel, 1.9, false);
    C3D_LightEnvLut(&lightEnv, GPU_LUT_FR, GPU_LUTINPUT_NV, false, &lutShittyFresnel);
    C3D_LightEnvFresnel(&lightEnv, GPU_PRI_SEC_ALPHA_FRESNEL);
    
    C3D_FVec lightVec = FVec4_New(0.5, 2.0, 0.0, 1.0);
    C3D_LightInit(&light, &lightEnv);
    C3D_LightColor(&light, 1.0, 1.0, 1.0);
    C3D_LightPosition(&light, &lightVec);
    
    env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_RGB, GPU_FRAGMENT_PRIMARY_COLOR, GPU_FRAGMENT_SECONDARY_COLOR, 0);
    C3D_TexEnvOpRgb(env, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_RGB, GPU_REPLACE);
    
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocProjection, &projection);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocModelview,  &modelview);
    
    //printf("Died before first drawcall");
    if(metaballVertCount != 0) {
        C3D_DrawArrays(GPU_TRIANGLES, 0, metaballVertCount);
    }
    //printf("Died after first drawcall");
    
    // Set up for statics
    float brightVal = sync_get_val(sync_bright, row);
    C3D_TexBind(0, &room_tex);
    C3D_LightColor(&light, 1.05 * brightVal, 0.7 * brightVal, 1.05 * brightVal);
    
    env = C3D_GetTexEnv(0);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, GPU_FRAGMENT_PRIMARY_COLOR, 0);
    C3D_TexEnvFunc(env, C3D_RGB, GPU_MODULATE);
    
    env = C3D_GetTexEnv(1);
    C3D_TexEnvInit(env);
    C3D_TexEnvSrc(env, C3D_Both, GPU_PREVIOUS, GPU_FRAGMENT_SECONDARY_COLOR, 0);
    C3D_TexEnvFunc(env, C3D_RGB, GPU_ADD);
    
    C3D_CullFace(GPU_CULL_BACK_CCW);
    C3D_LightEnvMaterial(&lightEnv, &lightMaterialDark);
    C3D_DrawArrays(GPU_TRIANGLES, MAX_METABALL_VERTS, numFacesSpaceRoom * 3);
    
     // GPU state for additive blend
    C3D_LightColor(&light, 1.0, 1.0, 1.0);
    C3D_CullFace(GPU_CULL_NONE);
    C3D_DepthTest(false, GPU_GEQUAL, GPU_WRITE_COLOR);
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_ONE, GPU_ONE, GPU_ONE, GPU_ONE);
    
    C3D_LightEnvBind(0);
    env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_Both, GPU_CONSTANT, GPU_CONSTANT, 0);
    C3D_TexEnvColor(env, 0x11330011);
    C3D_TexEnvOpRgb(env, GPU_TEVOP_RGB_SRC_COLOR, 0, 0);
    C3D_TexEnvOpAlpha(env, GPU_TEVOP_A_SRC_ALPHA, 0, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_ADD);
    
    Mtx_Scale(&modelview, 1.15, 1.15, 1.15);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocModelview,  &modelview);
    
    if(metaballVertCount != 0) {
        C3D_DrawArrays(GPU_TRIANGLES, 0, metaballVertCount);
    }
}

static int framecnt = 0;
void effectSunRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float row) {
    // Update state
    startPerfCounter(0);
    effectSunUpdate(row);
    //printf("A");
    // Set up for drawing
    resetShadeEnv();
    //printf("B");
    attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3); // v0 = position
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2); // v1 = texcoord
    AttrInfo_AddLoader(attrInfo, 2, GPU_FLOAT, 3); // v2 = normal        
    //printf("C");
    // Textures
//     C3D_TexBind(0, 0);    
    //printf("D");
    stopPerfCounter(0);
    //printf("E\n");
    // Start frame
    //C3D_FrameBegin(C3D_FRAME_NONBLOCK);
    //gspWaitForPPF();
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    //printf("F");
    C3D_RenderTargetClear(targetLeft, C3D_CLEAR_ALL, 0x000000FF, 0);
    C3D_RenderTargetClear(targetRight, C3D_CLEAR_ALL, 0x000000FF, 0);   
    
    // Left eye
    C3D_FrameDrawOn(targetLeft);
    effectSunDraw(-iod, row);
    
    if(iod > 0.0) {
        // Right eye
        C3D_FrameDrawOn(targetRight);
        effectSunDraw(iod, row);
        C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);    
        fade();
    }
    
    // Fade left eye here to avoid double draw setup
    C3D_AlphaBlend(GPU_BLEND_ADD, GPU_BLEND_ADD, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA, GPU_SRC_ALPHA, GPU_ONE_MINUS_SRC_ALPHA);    
    C3D_FrameDrawOn(targetLeft);
    fade();
    
    // Ready to flip
    C3D_FrameEnd(0);
    //printf("Died after frame end");
    
    framecnt++;
    if(framecnt % 120 == 0) {
        //printf("%d v, %f g / %f c\n", metaballVertCount, C3D_GetDrawingTime(), C3D_GetProcessingTime());
        //printf("pc 0: %f, 1: %f, 2: %f, 3: %f\n", readPerfCounter(0), readPerfCounter(1), readPerfCounter(2), readPerfCounter(3));
    }
}

void effectSunExit() {
//     gspWaitForP3D();
//     gspWaitForPPF();
// 
//     printf("Free tex\n");
    // Free textures
    C3D_TexDelete(&skybox_tex);
    C3D_TexDelete(&room_tex);
//     printf("A\n");
//     waitForA();
    // Free allocated memory
    linearFree(metaballVBO);
//     printf("B\n");
//     waitForA();
    
    free(marchingCubesGrid);
//     printf("C\n");
//     waitForA();
    
    free(marchingCubesNormals);
//     printf("D\n");
//     waitForA();
    
    free(marchingCubesMarkGrid);
//     printf("E\n");
//     waitForA();
    
    free(marchingCubesQueue);
//     printf("F\n");
//     waitForA();
    
}
