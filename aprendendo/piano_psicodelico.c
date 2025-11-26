#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <math.h>
#include <conio.h>
#include <time.h>
#include <stdint.h>  // ⬅️ ADICIONAR PARA Uint32, Uint8
#include <string.h>  // ⬅️ ADICIONAR PARA strcpy, strc

#define BUFFER_SIZE (44100 / 20)
#define MAX_VOICES 5
#define PI 3.14159265358979323

// Definir tipos se não existirem
#ifndef UINT32
typedef uint32_t Uint32;
#endif
#ifndef UINT8
typedef uint8_t Uint8;
#endif

// Constantes para a visualização
#define VIZ_WIDTH 800
#define VIZ_HEIGHT 600
#define MAX_PARTICLES 500

typedef struct {
    float freq;
    int active;
    double phase;
    int key;
    float amplitude;
    Uint32 start_time;
} Voice;

Voice voices[MAX_VOICES] = {0};

// Estruturas para visualização
typedef struct {
    float x, y;
    float vx, vy;
    float life;
    float max_life;
    int key;
    Uint8 r, g, b;
} Particle;

typedef struct {
    HWND hwnd;
    HDC hdc;
    HDC buffer_dc;
    HBITMAP buffer_bitmap;
    BITMAPINFO bmi;
    void* pixel_buffer;
    int width, height;
    Particle particles[MAX_PARTICLES];
    int particle_count;
    Uint32 start_time;
} Visualizer;

//notas
typedef struct {
    int key;
    float freq;
    const char* name;
} KeyMap;

KeyMap keymap[] = {
    {'A', 261.63, "DO"},   // DO
    {'S', 293.66, "RE"},   // RE
    {'D', 329.63, "MI"},   // MI
    {'F', 349.23, "FA"},   // FA
    {'G', 392.00, "SOL"},  // SOL
    {'H', 440.00, "LA"},   // LA
    {'J', 493.88, "SI"},   // SI
    {'K', 261.63*2, "DO↑"}, // DO ↑
    {'Q', 261.63/2, "DO↓"}, // DO ↓
    {'W', 293.66/2, "RE↓"}, // RE ↓
    {'E', 329.63/2, "MI↓"}, // MI ↓
    {'R', 349.23/2, "FA↓"}, // FA ↓
    {'T', 392.00/2, "SOL↓"},// SOL ↓
    {'Y', 440.00/2, "LA↓"}, // LA ↓
    {'U', 493.88/2, "SI↓"}, // SI ↓
    {'Z', 293.66*2, "RE↑"}, // RE ↑
    {'X', 329.63*2, "MI↑"}, // MI ↑
    {'C', 349.23*2, "FA↑"}, // FA ↑
    {'V', 392.00*2, "SOL↑"},// SOL ↑
    {'B', 440.00*2, "LA↑"}, // LA ↑
    {'N', 493.88*2, "SI↑"}, // SI ↑
    {'M', 261.63*3, "DO↑↑"},// DO ↑↑
};
int KEYMAP_SIZE = sizeof(keymap) / sizeof(KeyMap);

// Funções de visualização
Visualizer* init_visualizer() {
    Visualizer* viz = (Visualizer*)malloc(sizeof(Visualizer));
    
    // Registrar classe da janela
    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.lpszClassName = "PianoViz";
    
    RegisterClassEx(&wc);
    
    // Criar janela
    viz->hwnd = CreateWindowEx(
        0, "PianoViz", "🎹 Piano Psicodelico - Visualizacao",
        WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, VIZ_WIDTH, VIZ_HEIGHT,
        NULL, NULL, GetModuleHandle(NULL), NULL
    );
    
    viz->hdc = GetDC(viz->hwnd);
    viz->buffer_dc = CreateCompatibleDC(viz->hdc);
    
    // Configurar bitmap buffer
    viz->width = VIZ_WIDTH;
    viz->height = VIZ_HEIGHT;
    
    viz->bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    viz->bmi.bmiHeader.biWidth = viz->width;
    viz->bmi.bmiHeader.biHeight = -viz->height; // Top-down
    viz->bmi.bmiHeader.biPlanes = 1;
    viz->bmi.bmiHeader.biBitCount = 32;
    viz->bmi.bmiHeader.biCompression = BI_RGB;
    
    viz->buffer_bitmap = CreateDIBSection(viz->buffer_dc, &viz->bmi, DIB_RGB_COLORS, &viz->pixel_buffer, NULL, 0);
    SelectObject(viz->buffer_dc, viz->buffer_bitmap);
    
    viz->particle_count = 0;
    viz->start_time = GetTickCount();
    
    return viz;
}

void cleanup_visualizer(Visualizer* viz) {
    if (viz) {
        DeleteObject(viz->buffer_bitmap);
        DeleteDC(viz->buffer_dc);
        ReleaseDC(viz->hwnd, viz->hdc);
        DestroyWindow(viz->hwnd);
        free(viz);
    }
}

void add_particle(Visualizer* viz, int key, float freq) {
    if (viz->particle_count >= MAX_PARTICLES) return;
    
    Particle* p = &viz->particles[viz->particle_count++];
    p->x = VIZ_WIDTH / 2;
    p->y = VIZ_HEIGHT / 2;
    p->vx = (rand() % 200 - 100) / 50.0f;
    p->vy = (rand() % 200 - 100) / 50.0f;
    p->life = 1.0f;
    p->max_life = 2.0f + (rand() % 100) / 100.0f;
    p->key = key;
    
    // Cor baseada na frequência
    float hue = fmod(freq * 0.1f, 1.0f);
    float r, g, b;
    
    // HSL to RGB (simplificado)
    hue *= 6.0f;
    int i = (int)hue;
    float f = hue - i;
    float q = 1.0f - f;
    
    switch (i % 6) {
        case 0: r = 1; g = f; b = 0; break;
        case 1: r = q; g = 1; b = 0; break;
        case 2: r = 0; g = 1; b = f; break;
        case 3: r = 0; g = q; b = 1; break;
        case 4: r = f; g = 0; b = 1; break;
        default: r = 1; g = 0; b = q; break;
    }
    
    p->r = (Uint8)(r * 255);
    p->g = (Uint8)(g * 255);
    p->b = (Uint8)(b * 255);
}

void update_particles(Visualizer* viz) {
    for (int i = 0; i < viz->particle_count; i++) {
        Particle* p = &viz->particles[i];
        p->x += p->vx;
        p->y += p->vy;
        p->vy += 0.05f; // gravidade
        p->life -= 1.0f / 60.0f; // assumindo 60 FPS
        
        // Remover partículas mortas
        if (p->life <= 0 || p->x < 0 || p->x >= VIZ_WIDTH || p->y < 0 || p->y >= VIZ_HEIGHT) {
            viz->particles[i] = viz->particles[--viz->particle_count];
            i--;
        }
    }
}

void draw_circular_waves(Visualizer* viz) {
    Uint32 current_time = GetTickCount();
    float time = (current_time - viz->start_time) / 1000.0f;
    
    for (int v = 0; v < MAX_VOICES; v++) {
        if (voices[v].active) {
            float age = (current_time - voices[v].start_time) / 1000.0f;
            if (age > 3.0f) continue;
            
            // Cor baseada na tecla
            Uint8 r = (voices[v].key * 50) % 255;
            Uint8 g = (voices[v].key * 70) % 255;
            Uint8 b = (voices[v].key * 90) % 255;
            
            // Desenhar ondas concêntricas
            for (int radius = 20; radius < 300; radius += 25) {
                float wave_age = age - radius * 0.01f;
                if (wave_age > 0 && wave_age < 2.0f) {
                    int alpha = (int)(255 * (1.0f - wave_age / 2.0f));
                    
                    HPEN pen = CreatePen(PS_SOLID, 2, RGB(r, g, b));
                    SelectObject(viz->buffer_dc, pen);
                    
                    // Desenhar círculo com distorção
                    int center_x = VIZ_WIDTH / 2;
                    int center_y = VIZ_HEIGHT / 2;
                    
                    for (int angle = 0; angle < 360; angle += 5) {
                        float rad = angle * PI / 180.0f;
                        float distortion = sin(wave_age * 5 + angle * 0.1f) * 10.0f;
                        int x = center_x + (radius + distortion) * cos(rad);
                        int y = center_y + (radius + distortion) * sin(rad);
                        
                        if (angle == 0) {
                            MoveToEx(viz->buffer_dc, x, y, NULL);
                        } else {
                            LineTo(viz->buffer_dc, x, y);
                        }
                    }
                    LineTo(viz->buffer_dc, center_x + radius, center_y);
                    
                    DeleteObject(pen);
                }
            }
        }
    }
}

void draw_spectrum(Visualizer* viz) {
    int bar_width = VIZ_WIDTH / MAX_VOICES;
    
    for (int v = 0; v < MAX_VOICES; v++) {
        if (voices[v].active) {
            int height = (int)(voices[v].amplitude * VIZ_HEIGHT * 0.8f);
            int x = v * bar_width;
            int y = VIZ_HEIGHT - height;
            
            // Cor gradiente baseada na frequência
            int color_intensity = (int)(voices[v].freq / 100.0f) % 255;
            HBRUSH brush = CreateSolidBrush(RGB(color_intensity, 255 - color_intensity, 128));
            SelectObject(viz->buffer_dc, brush);
            
            Rectangle(viz->buffer_dc, x, y, x + bar_width - 2, VIZ_HEIGHT);
            DeleteObject(brush);
        }
    }
}

void draw_particles(Visualizer* viz) {
    for (int i = 0; i < viz->particle_count; i++) {
        Particle* p = &viz->particles[i];
        int alpha = (int)(p->life / p->max_life * 255);
        
        HBRUSH brush = CreateSolidBrush(RGB(p->r, p->g, p->b));
        SelectObject(viz->buffer_dc, brush);
        
        int size = (int)(p->life * 10 + 2);
        Ellipse(viz->buffer_dc, 
               (int)p->x - size, (int)p->y - size,
               (int)p->x + size, (int)p->y + size);
        
        DeleteObject(brush);
    }
}

void render_visualization(Visualizer* viz) {
    // Limpar tela
    RECT clear_rect = {0, 0, VIZ_WIDTH, VIZ_HEIGHT};
    HBRUSH black_brush = CreateSolidBrush(RGB(0, 0, 0));
    FillRect(viz->buffer_dc, &clear_rect, black_brush);
    DeleteObject(black_brush);
    
    // Atualizar e desenhar componentes
    update_particles(viz);
    draw_circular_waves(viz);
    draw_spectrum(viz);
    draw_particles(viz);
    
    // Desenhar informações do piano
    HFONT font = CreateFont(20, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                           DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                           DEFAULT_QUALITY, DEFAULT_PITCH, "Arial");
    SelectObject(viz->buffer_dc, font);
    SetTextColor(viz->buffer_dc, RGB(255, 255, 255));
    SetBkMode(viz->buffer_dc, TRANSPARENT);
    
    // Mostrar teclas ativas
    char info[256] = {0};
    strcpy(info, "Teclas ativas: ");
    for (int v = 0; v < MAX_VOICES; v++) {
        if (voices[v].active) {
            for (int k = 0; k < KEYMAP_SIZE; k++) {
                if (keymap[k].key == voices[v].key) {
                    strcat(info, keymap[k].name);
                    strcat(info, " ");
                    break;
                }
            }
        }
    }
    
    wchar_t winfo[256];
    mbstowcs(winfo, info, 256);
    TextOutA(viz->buffer_dc, 10, 10, info, strlen(info));
    
    DeleteObject(font);
    
    // Copiar buffer para tela
    BitBlt(viz->hdc, 0, 0, VIZ_WIDTH, VIZ_HEIGHT, viz->buffer_dc, 0, 0, SRCCOPY);
}

void activate_voice(int key, float freq) {
    for (int v = 0; v < MAX_VOICES; v++) {
        if (voices[v].active && voices[v].key == key) {
            voices[v].amplitude = 1.0f;
            return;
        }
    }

    for (int v = 0; v < MAX_VOICES; v++){
        if(!voices[v].active){
            voices[v].freq = freq;
            voices[v].phase = 0;
            voices[v].active = 1;
            voices[v].key = key;
            voices[v].amplitude = 1.0f;
            voices[v].start_time = GetTickCount();
            return;
        }
    }
}

void deactivate_voice(int key) {
    for (int v = 0; v < MAX_VOICES; v++) {
        if (voices[v].active && voices[v].key == key) {
            voices[v].active = 0;
            voices[v].freq = 0.0f;
            voices[v].phase = 0.0;
            voices[v].key = 0;
            voices[v].amplitude = 0.0f;
            return;
        }
    }
}

static inline double clamp_double(double v, double lo, double hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

int main(){
    srand((unsigned int)time(NULL));
    
    // Inicializar visualização
    Visualizer* viz = init_visualizer();
    printf("Piano polifonico com visualização psicodélica!\n");
   
    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = 44100;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    HWAVEOUT hWave;
    if (waveOutOpen(&hWave, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        fprintf(stderr, "Erro ao abrir device de audio\n");
        return 1;
    }

    int play_type = 0;
    int volume = 28000;
    float duty_cycle = 0.3;

    printf("Teclas: A S D F G H J K Q W E R T Y U Z X C V B N M\n");
    printf("1=Sine 2=Square 3=Saw 4=Triangle 5=SquareDuty\n");
    printf("UP/DOWN volume, LEFT/RIGHT duty, ESC para sair\n");

    short buffer[BUFFER_SIZE];
    Uint32 last_viz_update = 0;

    while (1) {
        // Processar mensagens do Windows (para a janela não travar)
        MSG msg;
        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT) {
                goto cleanup;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (_kbhit()){
            char tecla = _getch();
            switch (tecla) {
                case '1': play_type = 0; printf("\rmodo sine        "); break;
                case '2': play_type = 1; printf("\rmodo quadrado      "); break;
                case '3': play_type = 2; printf("\rmodo serra       "); break;
                case '4': play_type = 3; printf("\rmodo triangle         "); break;
                case '5': play_type = 4; printf("\rmodo quadrado duty   "); break;
            }
        }

        if (GetAsyncKeyState(VK_UP) & 0x8000) {
            volume += 500;
            if (volume > 32767) volume = 32767;
            printf("\rVolume: %d   ", volume);
            Sleep(80);
        }
        if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
            volume -= 500;
            if (volume < 1000) volume = 1000;
            printf("\rVolume: %d   ", volume);
            Sleep(80);
        }
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
            duty_cycle += 0.05f;
            if (duty_cycle > 0.95f) duty_cycle = 0.95f;
            printf("\rduty: %.2f   ", duty_cycle);
            Sleep(150);
        }
        if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
            duty_cycle -= 0.05f;
            if (duty_cycle < 0.05f) duty_cycle = 0.05f;
            printf("\rduty: %.2f   ", duty_cycle);
            Sleep(150);
        }
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) break;

        // Processar notas do teclado
        for (int i = 0; i < KEYMAP_SIZE; i++) {
            int k = keymap[i].key;
            float f = keymap[i].freq;
            
            if (GetAsyncKeyState(k) & 0x8000) {
                if (!voices[0].active || voices[0].key != k) { // Verificação simplificada
                    activate_voice(k, f);
                    add_particle(viz, k, f);
                }
            } else {
                deactivate_voice(k);
            }
        }

        // Gerar áudio
        for (int i = 0; i < BUFFER_SIZE; i++) {
            double mix = 0;
            int active_count = 0;

            for (int v = 0; v < MAX_VOICES; v++) {
                if (voices[v].active && voices[v].freq > 0.0f) {
                    active_count++;
                    
                    double sample = 0;
                    double p = voices[v].phase;

                    switch (play_type) {
                        case 0: sample = sin(2 * PI * p); break;
                        case 1: sample = (sin(2 * PI * p) > 0 ? 1 : -1); break;
                        case 2: sample = 2 * (p - floor(p + 0.5)); break;
                        case 3: sample = 2 * fabs(2 * (p - floor(p + 0.5))) - 1; break;
                        case 4: sample = (fmod(p, 1.0) < duty_cycle) ? 1 : -1; break;
                    }

                    mix += sample;
                    voices[v].phase += voices[v].freq / 44100.0;
                    if (voices[v].phase >= 1.0) voices[v].phase -= 1.0;
                    
                    // Atualizar amplitude para visualização
                    voices[v].amplitude = fabs(sample);
                }
            }

            if (active_count > 0) {
                mix /= sqrt(active_count);
            }

            double vol_scale = (double)volume / 32767.0;
            mix *= vol_scale;
            mix = clamp_double(mix, -1.0, 1.0);
            buffer[i] = (short)(mix * 32767.0);
        }

        // Tocar áudio
        WAVEHDR header;
        header.dwBufferLength = BUFFER_SIZE * sizeof(short);
        header.lpData = (LPSTR)buffer;
        header.dwFlags = 0;

        if (waveOutPrepareHeader(hWave, &header, sizeof(header)) == MMSYSERR_NOERROR) {
            waveOutWrite(hWave, &header, sizeof(header));
            while (!(header.dwFlags & WHDR_DONE)) {
                // Espera ativa
            }
            waveOutUnprepareHeader(hWave, &header, sizeof(header));
        }

        // Atualizar visualização (limitar a ~60 FPS)
        Uint32 current_time = GetTickCount();
        if (current_time - last_viz_update > 16) {
            render_visualization(viz);
            last_viz_update = current_time;
        }
    }

cleanup:
    waveOutReset(hWave);
    waveOutClose(hWave);
    cleanup_visualizer(viz);
    return 0;
}