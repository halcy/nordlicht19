#include "Tools.h"
#include "vshader_flat_shbin.h"
#include "Perlin.h"

// Sync
struct sync_device *rocket;

int RandomInteger() {
    return rand();
}

int nnnoise(int x, int y, int oct) {
    if(oct == 0) {
        return(0);
    }
    else {
        srand(x * 100003 + y);
        int randVal = rand() % (1 << 16);
        return(randVal / (2 << oct) + nnnoise(x / 2, y / 2, oct - 1));
    }
}

float badFresnel(float input, float expo) {
    return(fmax(0.0, 0.9 - powf(fmax(input, 0.0), expo)));
}

float lutPosPower(float input, float expo) {
    return(powf(fmax(input, 0.0), expo));
}

float lutAbsLinear(float input, float offset) {
    return(fmax(0.0, offset + abs(input)));
}

float lutAbsInverseLinear(float input, float offset) {
    return(fmax(0.0, offset - abs(input)));
}

float lutOne(float input, float ignored) {
    return(1.0);
}

float lutZero(float input, float ignored) {
    return(0.0);
}

float lutSquaredDist(float input, float offset, float scale) {
    return((input*input) * scale + offset);
}

float lutDist(float input, float offset, float scale) {
    return(sqrt(input*input) * scale + offset);
}

static TickCounter perfCounters[4];
static int perfCounterInitialized = 0;
void startPerfCounter(int idx) {
//     osTickCounterStart(&perfCounters[idx]);
}

void stopPerfCounter(int idx) {
//     osTickCounterUpdate(&perfCounters[idx]);
}

float readPerfCounter(int idx) {
//     return osTickCounterRead(&perfCounters[idx]);
}

void resetShadeEnv() {
    C3D_LightEnvBind(0);
    
    C3D_TexEnv* env = C3D_GetTexEnv(0);
    C3D_TexEnvSrc(env, C3D_Both, GPU_TEXTURE0, 0, 0);
    C3D_TexEnvOpRgb(env, 0, 0, 0);
    C3D_TexEnvOpAlpha(env, 0, 0, 0);
    C3D_TexEnvFunc(env, C3D_Both, GPU_REPLACE);
    
    C3D_TexEnv* env2 = C3D_GetTexEnv(1);
    C3D_TexEnvSrc(env2, C3D_Both, GPU_PREVIOUS, 0, 0);
    C3D_TexEnvOpRgb(env2, 0, 0, 0);
    C3D_TexEnvOpAlpha(env2, 0, 0, 0);
    C3D_TexEnvFunc(env2, C3D_Both, GPU_REPLACE);
    
    C3D_TexEnv* env3 = C3D_GetTexEnv(2);
    C3D_TexEnvSrc(env3, C3D_Both, GPU_PREVIOUS, 0, 0);
    C3D_TexEnvOpRgb(env3, 0, 0, 0);
    C3D_TexEnvOpAlpha(env3, 0, 0, 0);
    C3D_TexEnvFunc(env3, C3D_Both, GPU_REPLACE);
    
    C3D_TexEnv* env4 = C3D_GetTexEnv(3);
    C3D_TexEnvSrc(env4, C3D_Both, GPU_PREVIOUS, 0, 0);
    C3D_TexEnvOpRgb(env4, 0, 0, 0);
    C3D_TexEnvOpAlpha(env4, 0, 0, 0);
    C3D_TexEnvFunc(env4, C3D_Both, GPU_REPLACE);
}

static DVLB_s* vshader_flat_dvlb;
static shaderProgram_s shaderProgramFlat;
static int shaderProgramFlatCompiled;
static C3D_Mtx projection;

void ensureFlatShader() {
    if(shaderProgramFlatCompiled == 0) {
        vshader_flat_dvlb = DVLB_ParseFile((u32*)vshader_flat_shbin, vshader_flat_shbin_size);
        shaderProgramInit(&shaderProgramFlat);
        shaderProgramSetVsh(&shaderProgramFlat, &vshader_flat_dvlb->DVLE[0]);
        shaderProgramFlatCompiled = 1;
    }
    C3D_BindProgram(&shaderProgramFlat);
    
    int uLocProjectionFlat = shaderInstanceGetUniformLocation(shaderProgramFlat.vertexShader, "projection");
    Mtx_OrthoTilt(&projection, 0.0, 400.0, 240.0, 0.0, 0.0, 1.0, true);
    C3D_FVUnifMtx4x4(GPU_VERTEX_SHADER, uLocProjectionFlat, &projection);
    
    C3D_AttrInfo* attrInfo = C3D_GetAttrInfo();
    AttrInfo_Init(attrInfo);
    AttrInfo_AddLoader(attrInfo, 0, GPU_FLOAT, 3);
    AttrInfo_AddLoader(attrInfo, 1, GPU_FLOAT, 2);
}

void fullscreenQuad(C3D_Tex texture, float iod, float iodmult) {
    ensureFlatShader();
    
    C3D_TexSetFilter(&texture, GPU_LINEAR, GPU_NEAREST);
    C3D_TexBind(0, &texture);
    
    resetShadeEnv();
    
    float preShift = iodmult > 0.0 ? 0.05 : 0.0;
    float textureLeft = -iod * iodmult + preShift;
    float textureRight = (float)SCREEN_WIDTH / (float)SCREEN_TEXTURE_WIDTH - iod * iodmult - preShift;
    float textureTop = 1.0 - (float)SCREEN_HEIGHT / (float)SCREEN_TEXTURE_HEIGHT + preShift * ((float)SCREEN_HEIGHT / (float)SCREEN_WIDTH);
    float textureBottom = 1.0 - preShift * ((float)SCREEN_HEIGHT / (float)SCREEN_WIDTH);
    
    // Turn off depth test as well as write
    C3D_DepthTest(false, GPU_GEQUAL, GPU_WRITE_COLOR);
    
    // Draw a textured quad directly
    C3D_ImmDrawBegin(GPU_TRIANGLES);
        
        C3D_ImmSendAttrib(0.0, 0.0, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureLeft, textureBottom, 0.0f, 0.0f);

        C3D_ImmSendAttrib(SCREEN_WIDTH, SCREEN_HEIGHT, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureRight, textureTop, 0.0f, 0.0f);

        C3D_ImmSendAttrib(SCREEN_WIDTH, 0.0, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureRight, textureBottom, 0.0f, 0.0f);

        C3D_ImmSendAttrib(0.0, 0.0, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureLeft, textureBottom, 0.0f, 0.0f);

        C3D_ImmSendAttrib(0.0, SCREEN_HEIGHT, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureLeft, textureTop, 0.0f, 0.0f);

        C3D_ImmSendAttrib(SCREEN_WIDTH, SCREEN_HEIGHT, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureRight, textureTop, 0.0f, 0.0f);

    C3D_ImmDrawEnd();
}

void fullscreenQuadHR(C3D_Tex texture, float iod, float iodmult) {
    ensureFlatShader();
    
    C3D_TexSetFilter(&texture, GPU_LINEAR, GPU_NEAREST);
    C3D_TexBind(0, &texture);
    
    resetShadeEnv();
    
//     float preShift = iodmult > 0.0 ? 0.05 : 0.0;
//     float textureLeft = -iod * iodmult + preShift;
//     float textureRight = (float)SCREEN_WIDTH / (float)SCREEN_TEXTURE_WIDTH - iod * iodmult - preShift;
//     float textureTop = 1.0 - (float)SCREEN_HEIGHT / (float)SCREEN_TEXTURE_HEIGHT + preShift * ((float)SCREEN_HEIGHT / (float)SCREEN_WIDTH);
//     float textureBottom = 1.0 - preShift * ((float)SCREEN_HEIGHT / (float)SCREEN_WIDTH);
//     printf("lrtb %f %f %f %f\n", textureLeft, textureRight, textureTop, textureBottom);
    float textureLeft = 0.0;
    float textureRight = 1.0;
    float textureTop = (256.0 - 153.0) / 256.0;
    float textureBottom = 1.0;
//     printf("lrtb %f %f %f %f\n", textureLeft, textureRight, textureTop, textureBottom);
    
    // Turn off depth test as well as write
    C3D_DepthTest(false, GPU_GEQUAL, GPU_WRITE_COLOR);
    
    // Draw a textured quad directly
    C3D_ImmDrawBegin(GPU_TRIANGLES);
        
        C3D_ImmSendAttrib(0.0, 0.0, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureLeft, textureBottom, 0.0f, 0.0f);

        C3D_ImmSendAttrib(SCREEN_WIDTH, SCREEN_HEIGHT, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureRight, textureTop, 0.0f, 0.0f);

        C3D_ImmSendAttrib(SCREEN_WIDTH, 0.0, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureRight, textureBottom, 0.0f, 0.0f);

        C3D_ImmSendAttrib(0.0, 0.0, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureLeft, textureBottom, 0.0f, 0.0f);

        C3D_ImmSendAttrib(0.0, SCREEN_HEIGHT, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureLeft, textureTop, 0.0f, 0.0f);

        C3D_ImmSendAttrib(SCREEN_WIDTH, SCREEN_HEIGHT, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureRight, textureTop, 0.0f, 0.0f);

    C3D_ImmDrawEnd();
}

void fullscreenQuadHRNS(C3D_Tex texture, float iod, float iodmult) {
    ensureFlatShader();
    
    C3D_TexSetFilter(&texture, GPU_LINEAR, GPU_NEAREST);
    C3D_TexBind(0, &texture);
    
    resetShadeEnv();
    
//     float preShift = iodmult > 0.0 ? 0.05 : 0.0;
//     float textureLeft = -iod * iodmult + preShift;
//     float textureRight = (float)SCREEN_WIDTH / (float)SCREEN_TEXTURE_WIDTH - iod * iodmult - preShift;
//     float textureTop = 1.0 - (float)SCREEN_HEIGHT / (float)SCREEN_TEXTURE_HEIGHT + preShift * ((float)SCREEN_HEIGHT / (float)SCREEN_WIDTH);
//     float textureBottom = 1.0 - preShift * ((float)SCREEN_HEIGHT / (float)SCREEN_WIDTH);
//     printf("lrtb %f %f %f %f\n", textureLeft, textureRight, textureTop, textureBottom);
    float textureLeft = -0.18;
    float textureRight = 1.0 + 0.18;
    float textureTop = (256.0 - 153.0) / 256.0 - 0.18;
    float textureBottom = 1.0 + 0.18;
//     printf("lrtb %f %f %f %f\n", textureLeft, textureRight, textureTop, textureBottom);
    
    // Turn off depth test as well as write
    C3D_DepthTest(false, GPU_GEQUAL, GPU_WRITE_COLOR);
    
    // Draw a textured quad directly
    C3D_ImmDrawBegin(GPU_TRIANGLES);
        
        C3D_ImmSendAttrib(0.0, 0.0, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureLeft, textureBottom, 0.0f, 0.0f);

        C3D_ImmSendAttrib(SCREEN_WIDTH, SCREEN_HEIGHT, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureRight, textureTop, 0.0f, 0.0f);

        C3D_ImmSendAttrib(SCREEN_WIDTH, 0.0, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureRight, textureBottom, 0.0f, 0.0f);

        C3D_ImmSendAttrib(0.0, 0.0, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureLeft, textureBottom, 0.0f, 0.0f);

        C3D_ImmSendAttrib(0.0, SCREEN_HEIGHT, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureLeft, textureTop, 0.0f, 0.0f);

        C3D_ImmSendAttrib(SCREEN_WIDTH, SCREEN_HEIGHT, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureRight, textureTop, 0.0f, 0.0f);

    C3D_ImmDrawEnd();
}

void fullscreenQuadFlat(C3D_Tex texture) {
    ensureFlatShader();

    C3D_TexSetFilter(&texture, GPU_LINEAR, GPU_NEAREST);
    C3D_TexBind(0, &texture);
    
    resetShadeEnv();
    
    float textureLeft = 0.0;
    float textureRight = 1.0;
    float textureTop = 0.0;
    float textureBottom = 1.0;
    
    // Turn off depth test as well as write
    C3D_DepthTest(false, GPU_GEQUAL, GPU_WRITE_COLOR);
    
    // Draw a textured quad directly
    C3D_ImmDrawBegin(GPU_TRIANGLES);
        
        C3D_ImmSendAttrib(0.0, 0.0, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureRight, textureBottom, 0.0f, 0.0f); 

        C3D_ImmSendAttrib(SCREEN_WIDTH, SCREEN_HEIGHT, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureLeft, textureTop, 0.0f, 0.0f);

        C3D_ImmSendAttrib(SCREEN_WIDTH, 0.0, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureRight, textureTop, 0.0f, 0.0f);

        C3D_ImmSendAttrib(0.0, 0.0, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureRight, textureBottom, 0.0f, 0.0f);

        C3D_ImmSendAttrib(0.0, SCREEN_HEIGHT, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureLeft, textureBottom, 0.0f, 0.0f);

        C3D_ImmSendAttrib(SCREEN_WIDTH, SCREEN_HEIGHT, 0.5f, 0.0f);
        C3D_ImmSendAttrib(textureLeft, textureTop, 0.0f, 0.0f);

    C3D_ImmDrawEnd();
}

void fullscreenQuadGlitch(C3D_Tex texture, int parts, float time, float amount) {
    ensureFlatShader();

    C3D_TexSetFilter(&texture, GPU_LINEAR, GPU_NEAREST);
    C3D_TexBind(0, &texture);
    
    resetShadeEnv();
    
    // Turn off depth test as well as write
    C3D_DepthTest(false, GPU_GEQUAL, GPU_WRITE_COLOR);
    
    for(int i = 0; i < parts; i++) {
        float quadTop = ((float)i / (float)parts) * SCREEN_HEIGHT;
        float quadBottom = ((float)(i+1) / (float)parts) * SCREEN_HEIGHT;
        float textureLeft =1.0 -  ((float)(i+1) / (float)parts);
        float textureRight = 1.0 - ((float)(i) / (float)parts);
        
        
        float distAmt = pow(sin(noise_at(i * 0.05 - time, 0.1, 0.1)) * amount, 3.0);
        float textureTop = 0.0 + distAmt;
        float textureBottom = 1.0 + distAmt;
        
        float distAmt2 = pow(sin(noise_at((i + 1) * 0.05 - time, 0.1, 0.1)) * amount, 3.0);
        float textureTopLeft = 0.0 + distAmt2;
        float textureBottomLeft = 1.0 + distAmt2;
        
        // Draw a textured quad directly
        C3D_ImmDrawBegin(GPU_TRIANGLES);
            C3D_ImmSendAttrib(0.0, quadTop, 0.5f, 0.0f);
            C3D_ImmSendAttrib(textureRight, textureBottom, 0.0f, 0.0f); 

            C3D_ImmSendAttrib(SCREEN_WIDTH, quadBottom, 0.5f, 0.0f);
            C3D_ImmSendAttrib(textureLeft, textureTopLeft, 0.0f, 0.0f);

            C3D_ImmSendAttrib(SCREEN_WIDTH, quadTop, 0.5f, 0.0f);
            C3D_ImmSendAttrib(textureRight, textureTop, 0.0f, 0.0f);

            C3D_ImmSendAttrib(0.0, quadTop, 0.5f, 0.0f);
            C3D_ImmSendAttrib(textureRight, textureBottom, 0.0f, 0.0f);

            C3D_ImmSendAttrib(0.0, quadBottom, 0.5f, 0.0f);
            C3D_ImmSendAttrib(textureLeft, textureBottomLeft, 0.0f, 0.0f);

            C3D_ImmSendAttrib(SCREEN_WIDTH, quadBottom, 0.5f, 0.0f);
            C3D_ImmSendAttrib(textureLeft, textureTopLeft, 0.0f, 0.0f);
        C3D_ImmDrawEnd();
    }
}

int32_t mulf32(int32_t a, int32_t b) {
        long long result = (long long)a * (long long)b;
        return (int32_t)(result >> 12);
}

int32_t divf32(int32_t a, int32_t b) {
        long long result = (long long)(a << 12) / (long long)b;
        return (int32_t)(result);
}

int32_t loadObject(int32_t numFaces, const index_triangle_t* faces, const init_vertex_t* vertices, const init_vertex_t* normals, const vec2_t* texcoords, vertex* vbo) {
    for(int f = 0; f < numFaces; f++) {
        for(int v = 0; v < 3; v++) {
            // Set up vertex
            uint32_t vertexIndex = faces[f].v[v];
            vbo[f * 3 + v].position[0] = vertices[vertexIndex].x;
            vbo[f * 3 + v].position[1] = vertices[vertexIndex].y;
            vbo[f * 3 + v].position[2] = vertices[vertexIndex].z;
            
            // Set normal to face normal
            vbo[f * 3 + v].normal[0] = normals[faces[f].v[3]].x;
            vbo[f * 3 + v].normal[1] = normals[faces[f].v[3]].y;
            vbo[f * 3 + v].normal[2] = normals[faces[f].v[3]].z;
            
            vbo[f * 3 + v].texcoord[0] = texcoords[vertexIndex].x;
            vbo[f * 3 + v].texcoord[1] = texcoords[vertexIndex].y; 
        }
    }
    return numFaces * 3;
}

int32_t loadObjectRigged(int32_t numFaces, const index_triangle_t* faces, const init_vertex_t* vertices, const init_vertex_t* normals, const vec2_t* texcoords, vertex* vbo) {
    for(int f = 0; f < numFaces; f++) {
        for(int v = 0; v < 3; v++) {
            // Set up vertex
            uint32_t vertexIndex = faces[f].v[v];
            vbo[f * 3 + v].position[0] = vertices[vertexIndex].x;
            vbo[f * 3 + v].position[1] = vertices[vertexIndex].y;
            vbo[f * 3 + v].position[2] = vertices[vertexIndex].z;
            
            // Set normal to face normal
            vbo[f * 3 + v].normal[0] = normals[faces[f].v[3]].x;
            vbo[f * 3 + v].normal[1] = normals[faces[f].v[3]].y;
            vbo[f * 3 + v].normal[2] = normals[faces[f].v[3]].z;
            
            vbo[f * 3 + v].texcoord[0] = texcoords[vertexIndex].x;
            vbo[f * 3 + v].texcoord[1] = texcoords[vertexIndex].y; 
        }
    }
    return numFaces * 3;
}


int32_t loadObject2to1(int32_t numFaces, const index_trianglepv_t* faces, const init_vertex_t* vertices, const init_vertex_t* normals, const vec2_t* texcoords, vertex* vbo) {
    for(int f = 0; f < numFaces; f++) {
        for(int v = 0; v < 3; v++) {
            // Set up vertex
            uint32_t vertexIndex = faces[f].v[v];
            vbo[f * 3 + v].position[0] = vertices[vertexIndex].x;
            vbo[f * 3 + v].position[1] = vertices[vertexIndex].y;
            vbo[f * 3 + v].position[2] = vertices[vertexIndex].z;
            
            // Set normals to vertex normals
            vbo[f * 3 + v].normal[0] = normals[faces[f].v[v+3]].x;
            vbo[f * 3 + v].normal[1] = normals[faces[f].v[v+3]].y;
            vbo[f * 3 + v].normal[2] = normals[faces[f].v[v+3]].z;

            // Set texcoords
            vbo[f * 3 + v].texcoord[0] = texcoords[faces[f].v[v+6]].x;
            vbo[f * 3 + v].texcoord[1] = texcoords[faces[f].v[v+6]].y;
        }
    }
    return numFaces * 3;
}

int32_t loadObject2(int32_t numFaces, const index_trianglepv_t* faces, const init_vertex_t* vertices, const init_vertex_t* normals, const vec2_t* texcoords, vertex2* vbo) {
    for(int f = 0; f < numFaces; f++) {
        for(int v = 0; v < 3; v++) {
            // Set up vertex
            uint32_t vertexIndex = faces[f].v[v];
            vbo[f * 3 + v].position[0] = vertices[vertexIndex].x;
            vbo[f * 3 + v].position[1] = vertices[vertexIndex].y;
            vbo[f * 3 + v].position[2] = vertices[vertexIndex].z;
            
            // Set normals to vertex normals
            vbo[f * 3 + v].normal[0] = normals[faces[f].v[v+3]].x;
            vbo[f * 3 + v].normal[1] = normals[faces[f].v[v+3]].y;
            vbo[f * 3 + v].normal[2] = normals[faces[f].v[v+3]].z;
            
            // Set tangents to vertex tangents
            vbo[f * 3 + v].tangent[0] = normals[faces[f].v[v+3]].x;
            vbo[f * 3 + v].tangent[1] = normals[faces[f].v[v+3]].y;
            vbo[f * 3 + v].tangent[2] = normals[faces[f].v[v+3]].z;            
            
            // Set texcoords
            vbo[f * 3 + v].texcoord[0] = texcoords[faces[f].v[v+6]].x;
            vbo[f * 3 + v].texcoord[1] = texcoords[faces[f].v[v+6]].y;
        }
    }
    return numFaces * 3;
}

// Helper function for loading a texture from a t3x file 
bool loadTex3DS(C3D_Tex* tex, C3D_TexCube* cube, const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        printf("Texture file not found: %s\n", path);
        return false;
    }
    
    Tex3DS_Texture t3x = Tex3DS_TextureImportStdio(f, tex, cube, true);
    fclose(f); 
    if (!t3x) {
        printf("Texture load failure on %s\n", path);
        return false; 
    }
    
    // Delete the t3x object since we don't need it
    Tex3DS_TextureFree(t3x);
    return true;
}

// Helper function for loading a texture from memory
bool loadTex3DSMem(C3D_Tex* tex, C3D_TexCube* cube, const void* data, size_t size) {
    Tex3DS_Texture t3x = Tex3DS_TextureImport(data, size, tex, cube, true);
    if (!t3x) {
        printf("Texture load failure on memory texture.\n");
        return false; 
    }
    
    // Delete the t3x object since we don't need it
    Tex3DS_TextureFree(t3x);
    return true;
}

extern C3D_Tex fade_tex;
extern float fadeVal;

void fade() {
    if(fadeVal > 0) {
        fullscreenQuad(fade_tex, 0.0, 0.0);
    }
}

u8* readFileMem(const char* fileName, u32* fileSize, bool linear) {
    FILE *fileptr;
    u8 *buffer;
    long filelen;

    fileptr = fopen(fileName, "rb");
    fseek(fileptr, 0, SEEK_END);
    *fileSize = ftell(fileptr);
    rewind(fileptr);
    
    if(linear) {
        buffer = (u8*)linearAlloc((1 + (*fileSize))*sizeof(u8));
    }
    else {
        buffer = (u8*)malloc((1 + (*fileSize))*sizeof(u8));
    }
    fread(buffer, *fileSize, 1, fileptr);
    fclose(fileptr);
    return(buffer);
}

// Wait for A B (good for debugging)
void waitForA() {
//     while(1) {
        hidScanInput();
//         u32 kDown = hidKeysDown();
//         if (kDown & KEY_A) {
//             break; // break in order to return to hbmenu
//         }
//     }
//     
//     while(1) {
        hidScanInput();
//         u32 kDown = hidKeysDown();
//         if (kDown & KEY_B) {
//             break; // break in order to return to hbmenu
//         }
//     }
}
