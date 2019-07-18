#include "Tools.h"
#include "Rocket/sync.h"

#include "Effects.h"

#define CLEAR_COLOR 0x555555FF

C3D_Tex fade_tex;
static Pixel* fadePixels;
static Bitmap fadeBitmap;
float fadeVal;

u32 music_bin_size;
u8* music_bin;

#define min(a, b) (((a)<(b))?(a):(b))
#define max(a, b) (((a)>(b))?(a):(b))

#define AUDIO_BUFSIZE 256

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

#ifdef SYNC_PLAYER
#define DEV_MODE
#endif

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
void audio_callback(void* ignored) {
    ignored;
    if(wave_buffer[fill_buffer].status == NDSP_WBUF_DONE && (sample_pos + AUDIO_BUFSIZE) * sizeof(int16_t) < music_bin_size) {
        if(audio_playing == 1) {
            sample_pos += AUDIO_BUFSIZE;
        }
        uint8_t *dest = (uint8_t*)wave_buffer[fill_buffer].data_pcm16;
        fseek(audioFile, (sample_pos - AUDIO_BUFSIZE) * sizeof(int16_t), SEEK_SET);
        fread(dest, AUDIO_BUFSIZE * sizeof(int16_t), 1, audioFile);
        //memcpy(dest, &music_bin[(sample_pos - AUDIO_BUFSIZE) * sizeof(int16_t)], AUDIO_BUFSIZE * sizeof(int16_t));
        DSP_FlushDataCache(dest, AUDIO_BUFSIZE * sizeof(int16_t));
        ndspChnWaveBufAdd(0, &wave_buffer[fill_buffer]);
        fill_buffer = !fill_buffer;
    }
}

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
//     printf("Music load ----- %d %d %d\n", linearSpaceFree(), vramSpaceFree(), mappableSpaceFree());
//     music_bin = readFileMem(, &music_bin_size, false);

    audioFile = fopen("romfs:/music2.bin", "rb");
    fseek(audioFile, 0, SEEK_END);
    music_bin_size = ftell(audioFile);
    rewind(audioFile);
    
    // Load textures
//     printf("Tex load ----- %d %d %d\n", linearSpaceFree(), vramSpaceFree(), mappableSpaceFree());
//     stars_bin = readFileMem("romfs:/stars.bin", &stars_bin_size, false);
//     tex_room_bin = readFileMem("romfs:/tex_room.bin", &tex_room_bin_size, false);
    
    // Rocket startup
#ifndef SYNC_PLAYER
    printf("Now socketing...\n");
    SOC_buffer = (u32*)memalign(SOC_ALIGN, SOC_BUFFERSIZE);
    socInit(SOC_buffer, SOC_BUFFERSIZE);
    
    rocket = sync_create_device("sdmc:/sync");
#else
    printf("Loading tracks from romfs...");
    rocket = sync_create_device("romfs:/sync");
#endif
    if(connect_rocket()) {
        return(0);
    }
    
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
    
    const struct sync_track* sync_fade = sync_get_track(rocket, "global.fade");;
    const struct sync_track* sync_effect = sync_get_track(rocket, "global.effect");;    
    
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
            printf("Spinwait complete.");
        }
    }
    
    // Start up first effect
    int current_effect = (int)sync_get_val(sync_effect, row + 0.01);
    printf("STARTUP @ %d\n", current_effect);
    effect_list[current_effect].init();
    
    int fc = 0;
    while (aptMainLoop()) {
        if(!DUMPFRAMES) {
            row = audio_get_row();
        }
        else {
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
//         printf("----- %d %d %d\n", linearSpaceFree(), vramSpaceFree(), mappableSpaceFree());
#ifndef DEV_MODE
        if(new_effect != -1 && new_effect != current_effect) {
            printf("EFFECT %d EXIT\n", current_effect);
            effect_list[current_effect].exit();
            printf("----- %d %d %d\n", linearSpaceFree(), vramSpaceFree(), mappableSpaceFree());
            current_effect = new_effect;
            printf("EFFECT %d INIT\n", current_effect);
            effect_list[current_effect].init();
        }
#endif

        fadeVal = sync_get_val(sync_fade, row);
//         printf("whaat\n");
        FillBitmap(&fadeBitmap, RGBAf(0.0, 0.0, 0.0, fadeVal));
        GSPGPU_FlushDataCache(fadePixels, 64 * 64 * sizeof(Pixel));
        GX_DisplayTransfer((u32*)fadePixels, GX_BUFFER_DIM(64, 64), (u32*)fade_tex.data, GX_BUFFER_DIM(64, 64), TEXTURE_TRANSFER_FLAGS);
        gspWaitForPPF();
//         printf("we in here\n");
        hidScanInput();
//         printf("why?\n");
        // Respond to user input
        u32 kDown = hidKeysDown();
        if (kDown & KEY_START) {
            break; // break in order to return to hbmenu
        }  
        float slider = osGet3DSliderState();
        float iod = slider / 3.0;
//         printf("okay %d?\n", current_effect);
        effect_list[current_effect].render(targetLeft, targetRight, iod, row);
//         printf("hah\n");
        if(DUMPFRAMES) {
            gspWaitForP3D();
            gspWaitForPPF();
            
            u8* fbl = gfxGetFramebuffer(GFX_TOP, GFX_LEFT, NULL, NULL);
            
            char fname[255];
            sprintf(fname, "3ds/frames/fb_left_%08d.raw", fc);
            
            FILE* file = fopen(fname,"w");
            fwrite(fbl, sizeof(int32_t), SCREEN_HEIGHT * SCREEN_WIDTH, file);
            fflush(file);
            fclose(file);
            
            if(DUMPFRAMES_3D) {
                u8* fbr = gfxGetFramebuffer(GFX_TOP, GFX_RIGHT, NULL, NULL);
                
                sprintf(fname, "3ds/frames/fb_right_%08d.raw", fc);
                
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
