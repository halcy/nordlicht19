/**
 * Nordlicht 2019 - Skate Station
 * 
 * SVatG 2019 ~ halcy / wrl
 **/

#include "Tools.h"
#include "Rocket/sync.h"

#include "Effects.h"

#define CLEAR_COLOR 0x555555FF

C3D_Tex fade_tex;
static Pixel* fadePixels;
static Bitmap fadeBitmap;
float fadeVal;

u8* music_bin;
u32 music_bin_size;

// u8* music_bin_play;
// u32 music_bin_play_block;
// 
// u8* music_bin_preload;
// u32 music_bin_preload_block;

#define min(a, b) (((a)<(b))?(a):(b))
#define max(a, b) (((a)>(b))?(a):(b))

#define AUDIO_BUFSIZE 512
#define AUDIO_BLOCKSIZE 16384

#define SONG_BPM 173.0
#define SONG_BPS (SONG_BPM / 60.0)
#define SONG_SPS 32000
#define SONG_SPB (SONG_SPS / SONG_BPS)

#define ROWS_PER_BEAT 8
#define SAMPLES_PER_ROW (SONG_SPB / ROWS_PER_BEAT)

int32_t sample_pos = 0;
ndspWaveBuf wave_buffer[2];
uint8_t fill_buffer = 0;
uint8_t audio_playing = 1;

double audio_get_row() {
    return (double)sample_pos / (double)SAMPLES_PER_ROW;
}

/*#ifdef SYNC_PLAYER
#define DEV_MODE
#endif*/

#ifndef SYNC_PLAYER
void audio_pause(void *ignored, int flag) {
   ignored;
   audio_playing = !flag;
}

void audio_set_row(void *ignored, int row) {
    ignored;
    sample_pos = row * SAMPLES_PER_ROW - AUDIO_BUFSIZE;
}

int audio_is_playing(void *d) {
    return audio_playing;
}

struct sync_cb rocket_callbakcks = {
    audio_pause,
    audio_set_row,
    audio_is_playing
};
#endif

#define ROCKET_HOST "10.1.1.132"
//#define ROCKET_HOST "127.0.0.1"
#define SOC_ALIGN 0x1000
#define SOC_BUFFERSIZE 0x100000

static uint32_t *SOC_buffer = NULL;

int connect_rocket() {
#ifndef SYNC_PLAYER
    while(sync_tcp_connect(rocket, ROCKET_HOST, SYNC_DEFAULT_PORT)) {
        printf("Didn't work, again...\n");
        hidScanInput();
        u32 kDown = hidKeysDown();
        if (kDown & KEY_START) {
            return(1);
        }
        svcSleepThread(1000*1000*1000);
    }
#endif
    return(0);
}

FILE *audioFile;
u8 audioTempBuf[AUDIO_BUFSIZE * 4];
void audio_callback(void* ignored) {
    ignored;
    if(wave_buffer[fill_buffer].status == NDSP_WBUF_DONE && (sample_pos + AUDIO_BUFSIZE) * sizeof(int16_t) < music_bin_size) {
        if(audio_playing == 1) {
            sample_pos += AUDIO_BUFSIZE;
        }
        uint8_t *dest = (uint8_t*)wave_buffer[fill_buffer].data_pcm16;
        
        // For audio streaming (WIP)
//         memcpy(dest, audioTempBuf, AUDIO_BUFSIZE * sizeof(int16_t));
//         wantAudioData = 1;
//         int block_id = (sample_pos - AUDIO_BUFSIZE) / AUDIO_BLOCKSIZE;
//         if(block_id != music_bin_play_block) {
//             printf("Copying audio %d != %d @ %d\n", block_id, music_bin_play_block, sample_pos);
//             memcpy(music_bin_play, music_bin_preload, AUDIO_BLOCKSIZE * sizeof(int16_t));
//             music_bin_play_block = block_id;
//         }
//         int play_pos = (sample_pos - AUDIO_BUFSIZE) % AUDIO_BLOCKSIZE;
        
//         memcpy(dest, &music_bin_play[play_pos * sizeof(int16_t)], AUDIO_BUFSIZE * sizeof(int16_t));
        memcpy(dest, &music_bin[(sample_pos - AUDIO_BUFSIZE) * sizeof(int16_t)], AUDIO_BUFSIZE * sizeof(int16_t));        
        DSP_FlushDataCache(dest, AUDIO_BUFSIZE * sizeof(int16_t));
        ndspChnWaveBufAdd(0, &wave_buffer[fill_buffer]);
        fill_buffer = !fill_buffer;
    }
}

#include <vshader_normalmapping_shbin.h>
#include <vshader_skybox_shbin.h>
#include <vshader_shbin.h>
#include <vshader_bones_shbin.h>

DVLB_s* vshader_dvlb;
DVLB_s* vshader_normalmapping_dvlb;
DVLB_s* vshader_bones_dvlb;
DVLB_s* vshader_skybox_dvlb;
shaderProgram_s shaderProgram;
shaderProgram_s shaderProgramNormalMapping;
shaderProgram_s shaderProgramBones;
shaderProgram_s shaderProgramSkybox;

// For audio streaming (WIP)
// void music_preload(int block_id) {
//     if(block_id != music_bin_preload_block) {
//         int load_pos = block_id * AUDIO_BLOCKSIZE;
// //         printf("Preloading block %d != %d @ %d\n", block_id, music_bin_preload_block, load_pos);
//         fseek(audioFile, load_pos * sizeof(int16_t), SEEK_SET);
//         fread(music_bin_preload, AUDIO_BLOCKSIZE * sizeof(int16_t), 1, audioFile);
//         music_bin_preload_block = block_id;
//     }
// }

int main() {    
    bool DUMPFRAMES = false;
    bool DUMPFRAMES_3D = false;

    // Set up effect sequence
    effect effect_list[10];
    effect_list[0].init = effectSun2Init;
    effect_list[0].render = effectSun2Render;
    effect_list[0].exit = effectSun2Exit;
    
    effect_list[1].init = effectSunInit;
    effect_list[1].render = effectSunRender;
    effect_list[1].exit = effectSunExit;
    
    effect_list[2].init = effectDanceInit;
    effect_list[2].render = effectDanceRender;
    effect_list[2].exit = effectDanceExit;
    
    effect_list[3].init = effectCoolCubeInit;
    effect_list[3].render = effectCoolCubeRender;
    effect_list[3].exit = effectCoolCubeExit;
    
    effect_list[4].init = effectTunnelInit;
    effect_list[4].render = effectTunnelRender;
    effect_list[4].exit = effectTunnelExit;
    
    effect_list[5].init = effectSunInit;
    effect_list[5].render = effectSunRender;
    effect_list[5].exit = effectSunExit;
    
    // Initialize graphics
    gfxInit(GSP_RGBA8_OES, GSP_BGR8_OES, false);
    gfxSet3D(true);
    consoleInit(GFX_BOTTOM, NULL);
    C3D_Init(C3D_DEFAULT_CMDBUF_SIZE);

    // Initialize the render target
    C3D_RenderTarget* targetLeft = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTarget* targetRight = C3D_RenderTargetCreate(240, 400, GPU_RB_RGBA8, GPU_RB_DEPTH24_STENCIL8);
    C3D_RenderTargetSetOutput(targetLeft, GFX_TOP, GFX_LEFT,  DISPLAY_TRANSFER_FLAGS);
    C3D_RenderTargetSetOutput(targetRight, GFX_TOP, GFX_RIGHT, DISPLAY_TRANSFER_FLAGS);

    fadePixels = (Pixel*)linearAlloc(64 * 64 * sizeof(Pixel));
    InitialiseBitmap(&fadeBitmap, 64, 64, BytesPerRowForWidth(64), fadePixels);
    C3D_TexInit(&fade_tex, 64, 64, GPU_RGBA8);
 
    romfsInit();
    
    // Open music
    music_bin = readFileMem("romfs:/music2.bin", &music_bin_size, false);

    // Streaming audio loader
//     audioFile = fopen("romfs:/music2.bin", "rb");
//     fseek(audioFile, 0, SEEK_END);
//     music_bin_size = ftell(audioFile);
//     rewind(audioFile);
//     
//     music_bin_play = (u8*)malloc(65536 * sizeof(int16_t));
//     music_bin_preload = (u8*)malloc(65536 * sizeof(int16_t));
//     music_preload(0);
    
    // Rocket startup
#ifndef SYNC_PLAYER
    printf("Now socketing...\n");
    SOC_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
    socInit(SOC_buffer, SOC_BUFFERSIZE);
    
    rocket = sync_create_device("sdmc:/sync");
#else
//     printf("Loading tracks from romfs...");
    rocket = sync_create_device("romfs:/sync");
#endif
    if(connect_rocket()) {
        return(0);
    }
    
    // Load shaders
    vshader_dvlb = DVLB_ParseFile((u32*)vshader_shbin, vshader_shbin_size);
    shaderProgramInit(&shaderProgram);
    shaderProgramSetVsh(&shaderProgram, &vshader_dvlb->DVLE[0]);

    vshader_skybox_dvlb = DVLB_ParseFile((u32*)vshader_skybox_shbin, vshader_skybox_shbin_size);
    shaderProgramInit(&shaderProgramSkybox);
    shaderProgramSetVsh(&shaderProgramSkybox, &vshader_skybox_dvlb->DVLE[0]);

    vshader_bones_dvlb = DVLB_ParseFile((u32*)vshader_bones_shbin, vshader_bones_shbin_size);
    shaderProgramInit(&shaderProgramBones);
    shaderProgramSetVsh(&shaderProgramBones, &vshader_bones_dvlb->DVLE[0]);
    
    vshader_normalmapping_dvlb = DVLB_ParseFile((u32*)vshader_normalmapping_shbin, vshader_normalmapping_shbin_size);
    shaderProgramInit(&shaderProgramNormalMapping);
    shaderProgramSetVsh(&shaderProgramNormalMapping, &vshader_normalmapping_dvlb->DVLE[0]);
    
//     printf("All loaded");
    
    // Sound on
    ndspInit();
    
    ndspSetOutputMode(NDSP_OUTPUT_STEREO);

    ndspChnSetInterp(0, NDSP_INTERP_LINEAR);
    ndspChnSetRate(0, SONG_SPS);
    ndspChnSetFormat(0, NDSP_FORMAT_MONO_PCM16);
    
    float mix[12];
    memset(mix, 0, sizeof(mix));
    mix[0] = 1.0;
    mix[1] = 1.0;    
    ndspChnSetMix(0, mix);

    uint8_t *audio_buffer = (uint8_t*)linearAlloc(AUDIO_BUFSIZE * sizeof(int16_t) * 2);
    memset(wave_buffer,0,sizeof(wave_buffer));
    wave_buffer[0].data_vaddr = &audio_buffer[0];
    wave_buffer[0].nsamples = AUDIO_BUFSIZE;
    wave_buffer[1].data_vaddr = &audio_buffer[AUDIO_BUFSIZE * sizeof(int16_t)];
    wave_buffer[1].nsamples = AUDIO_BUFSIZE;
    
    // Play music
    ndspSetCallback(&audio_callback, 0);
    ndspChnWaveBufAdd(0, &wave_buffer[0]);
    ndspChnWaveBufAdd(0, &wave_buffer[1]);
    
    // Get first row value
    double row = 0.0;  
    row = audio_get_row();    
#ifndef SYNC_PLAYER
    if(sync_update(rocket, (int)floor(row), &rocket_callbakcks, (void *)0)) {
        printf("Lost connection, retrying.\n");
        if(connect_rocket()) {
            return(0);
        }
    }
#endif
    
    for(int i = 0; i < 100000; i++) {
        if(i == 100000 - 1) {
            printf("");
        }
    }
    
    const struct sync_track* sync_fade = sync_get_track(rocket, "global.fade");;
    const struct sync_track* sync_effect = sync_get_track(rocket, "global.effect");;    
    
    // Start up first effect
    int current_effect = (int)sync_get_val(sync_effect, row + 0.01);
//     printf("STARTUP @ %d\n", current_effect);
    effect_list[current_effect].init();
    
    char* effect_texts[6] = {
      //"Nothing really special at the start - we"     
        "Effect 1: Logos and zoom in\n"
        "===========================\n"
        "Nothing really special at the start - \n"
        "we just zoom in really fast, show a few\n"
        "logos\n\n"
        "The space station uses normal mapping\n"
        "and the sun actually uses the 3DS\n"
        "procedural texturing unit to generate a\n"
        "nice detailed texture that is larger\n"
        "than what the 3DS could usually do.\n\n",
        "Effect 2: Metaballs\n"
        "===================\n"
        "Marching cubes'd and using central\n"
        "difference normals, trying to do as\n"
        "little work as we can get away with.\n"
        "Took forever to debug some memory\n"
        "issues here - remember to not access\n"
        "memory you have not malloc'd, kids.\n\n"
        "Rendered solid once and then again in\n"
        "an additive pass. There is also a \n"
        "subtle lighting effect to make it look\n"
        "like the balls are emitting light.\n\n",
        "Effect 3: Hyperspace\n"
        "====================\n"
        "The FOV of Effect 1 is animated. Not\n"
        "much to say about this other than that.\n\n"
        "I fixed a bug in this an hour after the\n"
        "deadline and had to re-framedump the\n"
        "part real quick.\n\n",
        "Effect 4: Greets Cube\n"
        "=====================\n"
        "Welcome to the Greet Cube. This is\n"
        "really mostly texture mapping with a\n"
        "dynamic, partially translucent texture\n"
        "with some nice glitchy interior in the\n"
        "model.\n\n"
        "The comet ribbons were added because I\n"
        "thought there wasn't enough going on.\n\n",
        "Effect 5: Bone Animation\n"
        "========================\n"
        "It's straight bone animation. The\n"
        "movement data was motion captured by\n"
        "me on a system I happened to have\n"
        "access to and then synchronized to the\n"
        "music after the fact.\n\n"
        "The background is a simple dynamic\n"
        "starfield texture.\n\n",
        "Effect 6: Lasertunnel\n"
        "=====================\n"
        "A tunnel segment with some dynamic\n"
        "geometry for the doors. There's always\n"
        "three segments visible at any given\n"
        " time. The lasers are recycle from our\n"
        "last release, but they look a lot\n"
        "cooler here, in my opinion.\n\n"
        "That's about it - I hope you enjoyed\n"
        "our release. If you want to know more,\n"
        "the source code is available:\n\n"
        " https://github.com/halcy/nordlicht19\n\n"
        "Now release 3DS prods, you cowards!\n"
    };
    
    int fc = 0;
    int eff_c = 0;
    printf(effect_texts[eff_c]);
    while (aptMainLoop()) {
//         music_preload(music_bin_play_block + 1);
        
        if(!DUMPFRAMES) {
            row = audio_get_row();
        }
        else {
            printf("Frame dump %d\n", fc);
            row = ((double)fc * (32000.0 / 30.0)) / (double)SAMPLES_PER_ROW;
        }
        
#ifndef SYNC_PLAYER
        if(sync_update(rocket, (int)floor(row), &rocket_callbakcks, (void *)0)) {
            printf("Lost connection, retrying.\n");
            if(connect_rocket()) {
                return(0);
            }
        }
#endif
        int new_effect = (int)sync_get_val(sync_effect, row);
#ifndef DEV_MODE
        if(new_effect != -1 && new_effect != current_effect) {
            effect_list[current_effect].exit();
            current_effect = new_effect;
            effect_list[current_effect].init();
            eff_c++;
            printf(effect_texts[eff_c]);
        }
#endif

        fadeVal = sync_get_val(sync_fade, row);
        FillBitmap(&fadeBitmap, RGBAf(0.0, 0.0, 0.0, fadeVal));
        GSPGPU_FlushDataCache(fadePixels, 64 * 64 * sizeof(Pixel));
        GX_DisplayTransfer((u32*)fadePixels, GX_BUFFER_DIM(64, 64), (u32*)fade_tex.data, GX_BUFFER_DIM(64, 64), TEXTURE_TRANSFER_FLAGS);
        gspWaitForPPF();
        hidScanInput();

        // Respond to user input
        u32 kDown = hidKeysDown();
        if (kDown & KEY_START) {
            break; // break in order to return to hbmenu
        }  
        float slider = osGet3DSliderState();
        float iod = slider / 3.0;

        effect_list[current_effect].render(targetLeft, targetRight, iod, row);

        if(DUMPFRAMES) {
            gspWaitForP3D();
            gspWaitForPPF();
            
            u8* fbl = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
            
            char fname[255];
            sprintf(fname, "3ds/frames2/fb_left_%08d.raw", fc);
            
            FILE* file = fopen(fname,"w");
            fwrite(fbl, sizeof(int32_t), SCREEN_HEIGHT * SCREEN_WIDTH, file);
            fflush(file);
            fclose(file);
            
            if(DUMPFRAMES_3D) {
                u8* fbr = gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL);
                
                sprintf(fname, "3ds/frames2/fb_right_%08d.raw", fc);
                
                file = fopen(fname,"w");
                fwrite(fbr, sizeof(int32_t), SCREEN_HEIGHT * SCREEN_WIDTH, file);
                fflush(file);
                fclose(file);
            }
        }
        
        fc++;   
    }
    
    linearFree(fadePixels);
    
    // Sound off
    ndspExit();
    linearFree(audio_buffer);
    
    // Deinitialize graphics
    socExit();
    C3D_Fini();
    gfxExit();
    
    return 0;
}
