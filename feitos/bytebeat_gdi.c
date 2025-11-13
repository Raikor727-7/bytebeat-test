// bytebeat_gdi.c
// Bytebeat em tempo real + visual GDI overlay transparente (Windows)
// Compilar: gcc bytebeat_gdi.c -o bytebeat_gdi -lwinmm -lgdi32
// Observação: execute em um terminal para ver logs. Feche com ESC.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <math.h>
#include <stdint.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "gdi32.lib")

#define RATE 8000
#define VIS_W  1024
#define VIS_H  600
#define FPS 60

// Shared state
static volatile int running = 1;
static volatile unsigned long global_t = 0;

// Audio waveout handle
static HWAVEOUT hWaveOut = NULL;

// Convert HSV to 32-bit ARGB (A=alpha)
static uint32_t hsv_to_argb(float h, float s, float v, float alpha) {
    float r, g, b;
    int i = (int)floor(h * 6.0f);
    float f = h * 6.0f - i;
    float p = v * (1.0f - s);
    float q = v * (1.0f - f * s);
    float t = v * (1.0f - (1.0f - f) * s);
    switch (i % 6) {
        case 0: r = v; g = t; b = p; break;
        case 1: r = q; g = v; b = p; break;
        case 2: r = p; g = v; b = t; break;
        case 3: r = p; g = q; b = v; break;
        case 4: r = t; g = p; b = v; break;
        default: r = v; g = p; b = q; break;
    }
    uint8_t R = (uint8_t)(fminf(1.0f, r) * 255.0f);
    uint8_t G = (uint8_t)(fminf(1.0f, g) * 255.0f);
    uint8_t B = (uint8_t)(fminf(1.0f, b) * 255.0f);
    uint8_t A = (uint8_t)(fminf(1.0f, alpha) * 255.0f);
    return (A << 24) | (R << 16) | (G << 8) | B;
}

// Thread: audio generating + filling buffer continuously
DWORD WINAPI audio_thread_func(LPVOID param) {
    // we'll generate 1-second buffers (RATE samples) like antes
    unsigned char *buffer = (unsigned char*)malloc(RATE);
    if (!buffer) return 0;

    while (running) {
        // generate RATE samples
        for (unsigned long t = 0; t < RATE; ++t) {
            unsigned long tt = global_t + t; // relative t
            // ----- Modifique a expressão aqui para experimentar sonoridades -----
            unsigned char sample = (unsigned char)((tt * ((tt >> 5) | (tt >> 8))) >> (tt >> 16));
            // -------------------------------------------------------------------
            buffer[t] = sample;
        }

        // play buffer
        WAVEHDR header;
        ZeroMemory(&header, sizeof(header));
        header.lpData = (LPSTR)buffer;
        header.dwBufferLength = RATE;
        if (waveOutPrepareHeader(hWaveOut, &header, sizeof(header)) == MMSYSERR_NOERROR) {
            waveOutWrite(hWaveOut, &header, sizeof(header));
            // Wait until buffer played (approx 1s). But we also keep global_t advancing.
            // Instead of Sleep(1000), poll for unprepared -> simpler: Sleep a bit and continue.
            // We'll advance global_t by RATE and sleep ~1s.
            Sleep(1000 * RATE / RATE); // ~=1000ms
            waveOutUnprepareHeader(hWaveOut, &header, sizeof(header));
        } else {
            Sleep(20);
        }

        // advance global time
        global_t += RATE;
    }

    free(buffer);
    return 0;
}

// Create a layered, transparent, topmost window
static HWND create_overlay_window(HINSTANCE hInst, int width, int height) {
    const char *cls = "BytebeatOverlayClass";
    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = DefWindowProcA;
    wc.hInstance = hInst;
    wc.lpszClassName = cls;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    RegisterClassA(&wc);

    DWORD exStyle = WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOPMOST;
    DWORD style = WS_POPUP;

    HWND hwnd = CreateWindowExA(
        exStyle,
        cls,
        "Bytebeat Visualizer",
        style,
        100, 100, width, height,
        NULL, NULL, hInst, NULL
    );
    if (!hwnd) return NULL;

    // Make window click-through and transparent to mouse
    LONG_PTR ex = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    ex |= WS_EX_TOOLWINDOW; // hide from ALT+TAB
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, ex);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);
    return hwnd;
}

// Main visual loop: creates a DIB section, updates pixels and calls UpdateLayeredWindow
DWORD WINAPI visual_thread_func(LPVOID param) {
    HWND hwnd = (HWND)param;
    int width = VIS_W, height = VIS_H;

    // Create memory DCs and DIBSection for ARGB
    HDC screenDC = GetDC(NULL);
    HDC memDC = CreateCompatibleDC(screenDC);

    BITMAPINFO bmi;
    ZeroMemory(&bmi, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void *bits = NULL;
    HBITMAP dib = CreateDIBSection(memDC, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
    if (!dib) {
        DeleteDC(memDC);
        ReleaseDC(NULL, screenDC);
        return 0;
    }
    HGDIOBJ oldBmp = SelectObject(memDC, dib);

    // for visual reaction, we will sample a pseudo "energy" from global_t (approx)
    // but better: derive a moving "energy" using sin/cos + small randomization
    float phase = 0.0f;
    float hueOffset = 0.0f;

    // main loop
    const int frameMs = 1000 / FPS;
    while (running) {
        // compute energy (0..1) from global_t - create periodic pulses
        float t_sec = (float)global_t / (float)RATE;
        float energy = 0.5f + 0.5f * sinf(t_sec * 2.0f * 1.1f); // slow pulse
        // add a faster jitter
        energy += 0.2f * sinf(t_sec * 40.0f) * (sinf(t_sec * 1.0f) * 0.5f + 0.5f);
        if (energy < 0) energy = 0;
        if (energy > 1.5f) energy = 1.5f;

        // draw into bits
        uint32_t *px = (uint32_t*)bits;
        const float cx = (float)width * 0.5f;
        const float cy = (float)height * 0.5f;

        // generate patterns: radial vortex + moving sine waves + dynamic palette
        hueOffset += 0.0009f * (9.0f + energy);
        phase += 0.1f * (1.0f + energy * 2.0f);

        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // normalized coords -1..1
                float nx = ((float)x - cx) / cx;
                float ny = ((float)y - cy) / cy;
                float r = sqrtf(nx*nx + ny*ny); // radius
                float a = atan2f(ny, nx);       // angle

                // base pattern: swirling rings modulated by phase and energy
                float swirl = sinf(10.0f * r - phase * 1.9f + a * 7.0f);
                float rings = fabsf(sinf(300.0f * r - phase * 6.0f));
                float wave = sinf(8.0f * nx + phase * 2.0f) * 0.5f + 0.5f;

                // combine patterns with energy
                float value = (0.5f*swirl + 0.8f*rings + 0.6f*wave) * (0.7f + energy*0.8f);

                // add radial falloff (center brighter)
                value *= (1.0f - r*r);

                // color from hue + angle
                float hue = fmodf(hueOffset + 0.2f * a + value * 0.3f + 1.0f, 1.0f);
                float sat = 0.6f + 0.4f * value;
                float val = 0.2f + 0.9f * value;

                // alpha based on distance (more transparent near edges)
                float alpha = 0.6f + 0.4f * (1.0f - r);

                // small time-dependent flicker
                float flick = 0.5f + 0.5f * sinf(5.0f * (nx + ny) + phase * 0.7f + r * 10.0f);
                val *= (0.6f + 0.4f * flick);

                uint32_t color = hsv_to_argb(hue < 0 ? hue + 1.0f : hue, sat, val, alpha);

                px[y * width + x] = color;
            }
        }

        // prepare blend
        POINT ptSrc = {0, 0};
        SIZE sizeWnd = {width, height};
        POINT ptDst;
        RECT wr;
        GetWindowRect(hwnd, &wr);
        ptDst.x = wr.left;
        ptDst.y = wr.top;

        BLENDFUNCTION bf;
        bf.BlendOp = AC_SRC_OVER;
        bf.BlendFlags = 0;
        bf.SourceConstantAlpha = 255; // use per-pixel alpha
        bf.AlphaFormat = AC_SRC_ALPHA;

        HDC hdcWindow = GetDC(NULL);
        // UpdateLayeredWindow with our ARGB DIB in memDC
        UpdateLayeredWindow(hwnd, hdcWindow, &ptDst, &sizeWnd, memDC, &ptSrc, 0, &bf, ULW_ALPHA);
        ReleaseDC(NULL, hdcWindow);

        Sleep(frameMs);
    }

    // cleanup
    SelectObject(memDC, oldBmp);
    DeleteObject(dib);
    DeleteDC(memDC);
    ReleaseDC(NULL, screenDC);
    return 0;
}

int main(void) {
    // init wave format
    WAVEFORMATEX wfx;
    ZeroMemory(&wfx, sizeof(wfx));
    wfx.nSamplesPerSec = RATE;
    wfx.wBitsPerSample = 8;
    wfx.nChannels = 1;
    wfx.cbSize = 0;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        printf("Erro ao abrir dispositivo de áudio.\n");
        return 1;
    }

    HINSTANCE hInst = GetModuleHandle(NULL);
    HWND hwnd = create_overlay_window(hInst, VIS_W, VIS_H);
    if (!hwnd) {
        printf("Falha ao criar janela overlay.\n");
        waveOutClose(hWaveOut);
        return 1;
    }

    // create threads
    HANDLE hAudioThread = CreateThread(NULL, 0, audio_thread_func, NULL, 0, NULL);
    HANDLE hVisualThread = CreateThread(NULL, 0, visual_thread_func, hwnd, 0, NULL);

    printf("Bytebeat GDI overlay rodando. Pressione ESC para sair.\n");

    // Simple message loop: process ESC to quit; allow window to be moved by ALT+TAB etc.
    // We'll still pump messages so UpdateLayeredWindow works correctly.
    MSG msg;
    while (running) {
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_KEYDOWN) {
                if (msg.wParam == VK_ESCAPE) running = 0;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        // also check keyboard asynchronously (in case no messages)
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
            running = 0;
        }
        Sleep(10);
    }

    // cleanup: signal threads to stop and wait
    printf("Encerrando...\n");
    WaitForSingleObject(hAudioThread, 100);
    WaitForSingleObject(hVisualThread, 100);

    // close waveout
    waveOutClose(hWaveOut);

    // destroy window
    DestroyWindow(hwnd);
    return 0;
}
