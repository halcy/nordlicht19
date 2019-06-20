// Nordlicht demoparty
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "Tools.h"
#include <vshader_shbin.h>

const struct sync_track* sync_testval;

void effectTemplateInit() {
    // initialize everything here
    // Example of how to get a sync track
    sync_testval = sync_get_track(rocket, "template.testval");
}

void effectTemplateUpdate(float row) {
    // You can update textures here, but afterwards you have to display transfer and wait for PPF interrupt:
    // GSPGPU_FlushDataCache(screenPixels, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(Pixel));
    // GX_DisplayTransfer((u32*)screenPixels, GX_BUFFER_DIM(SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT), (u32*)screen_tex.data, GX_BUFFER_DIM(SCREEN_TEXTURE_WIDTH, SCREEN_TEXTURE_HEIGHT), TEXTURE_TRANSFER_FLAGS);
    // gspWaitForPPF();
    
    // Or vertices or any other state ofc.
    // How to get a value from a sync track:
    float testval = sync_get_val(sync_testval, row);
}

void effectTemplateDraw(float iod) {
    // Drawing code goes here
    // Example stereoscopic projection matrix
    C3D_Mtx projection;
    Mtx_PerspStereoTilt(&projection, 65.0f * M_PI / 180.0f, 400.0f / 240.0f, 0.2f, 500.0f, iod, 2.0f, false);
}

void effectTemplateRender(C3D_RenderTarget* targetLeft, C3D_RenderTarget* targetRight, float iod, float row) {
    // Update state
    effectTemplateUpdate(row);
    
    // Start frame
    C3D_FrameBegin(C3D_FRAME_SYNCDRAW);
    C3D_RenderTargetClear(targetLeft, C3D_CLEAR_ALL, 0, 0);
    C3D_RenderTargetClear(targetRight, C3D_CLEAR_ALL, 0, 0);   
    
    // Left eye
    C3D_FrameDrawOn(targetLeft);
    effectTemplateDraw(-iod);
    fade();
    
    if(iod > 0.0) {
        // Right eye
        C3D_FrameDrawOn(targetRight);
        effectTemplateDraw(iod);
        fade();
    }
    
    // Ready to flip
    C3D_FrameEnd(0);
}

void effectTemplateExit() {
   // Teardown here
}
