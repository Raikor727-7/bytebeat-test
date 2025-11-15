// piano_poly.c
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <math.h>
#include <conio.h>

#define SAMPLE_RATE 44100
#define BUFFER_SIZE (SAMPLE_RATE / 20) // 1/20s buffer
#define MAX_VOICES 5
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef struct {
    float freq;
    int active;
    double phase;   // 0..1
    int key;        // tecla que ativou a voice (VK code ou ASCII)
} Voice;

Voice voices[MAX_VOICES] = {0};

typedef struct {
    int key;
    float freq;
} KeyMap;

KeyMap keymap[] = {
    {'A', 261.63f},   // DO
    {'S', 293.66f},   // RE
    {'D', 329.63f},   // MI
    {'F', 349.23f},   // FA
    {'G', 392.00f},   // SOL
    {'H', 440.00f},   // LA
    {'J', 493.88f},   // SI
    {'K', 261.63f*2}, // DO ↑
    {'Q', 261.63f/2}, // DO ↓
    {'W', 293.66f/2}, // RE ↓
    {'E', 329.63f/2}, // MI ↓
    {'R', 349.23f/2}, // FA ↓
    {'T', 392.00f/2}, // SOL ↓
    {'Y', 440.00f/2}, // LA ↓
    {'U', 493.88f/2}, // SI ↓
    {'Z', 293.66f*2}, // RE ↑
    {'X', 329.63f*2}, // MI ↑
    {'C', 349.23f*2}, // FA ↑
    {'V', 392.00f*2}, // SOL ↑
    {'B', 440.00f*2}, // LA ↑
    {'N', 493.88f*2}, // SI ↑
    {'M', 261.63f*4}, // DO ↑↑
};
int KEYMAP_SIZE = sizeof(keymap) / sizeof(KeyMap);

// ativa uma voice apenas se não houver já uma voice para essa tecla
void activate_voice(int key, float freq) {
    // se já existe voice dessa tecla, não recriar
    for (int v = 0; v < MAX_VOICES; v++) {
        if (voices[v].active && voices[v].key == key) {
            return;
        }
    }
    // procura voice livre
    for (int v = 0; v < MAX_VOICES; v++) {
        if (!voices[v].active) {
            voices[v].active = 1;
            voices[v].freq = freq;
            voices[v].phase = 0.0;
            voices[v].key = key;
            return;
        }
    }
    // Se chegou aqui, não havia slot livre — drop note (poderia implementar voice-stealing)
}

// desativa apenas a voice correspondente à tecla
void deactivate_voice(int key) {
    for (int v = 0; v < MAX_VOICES; v++) {
        if (voices[v].active && voices[v].key == key) {
            voices[v].active = 0;
            voices[v].freq = 0.0f;
            voices[v].phase = 0.0;
            voices[v].key = 0;
            return;
        }
    }
}

// clamp helper
static inline double clamp_double(double v, double lo, double hi) {
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

int main(void) {
    // wave format
    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = SAMPLE_RATE;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    HWAVEOUT hWave;
    if (waveOutOpen(&hWave, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        fprintf(stderr, "Erro ao abrir device de audio\n");
        return 1;
    }

    int play_type = 0;      // 0 = sine, 1 = square, 2 = saw, 3 = triangle, 4 = square duty
    int volume = 20000;     // 0..32767
    float duty_cycle = 0.3f;

    printf("Piano polifonico - teclas: A S D F G H J K Q W E R T Y U Z X C V B N M\n");
    printf("1=Sine 2=Square 3=Saw 4=Triangle 5=SquareDuty  UP/DOWN volume, LEFT/RIGHT duty\n");
    printf("ESC para sair\n");

    // buffer de áudio (shorts)
    short buffer[BUFFER_SIZE];

    while (1) {
        // leitura de switches e teclas especiais (modo/volume/duty)
        if (_kbhit()) {
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

        // percorre keymap: ativa enquanto tecla pressionada, desativa quando solta
        for (int i = 0; i < KEYMAP_SIZE; i++) {
            int k = keymap[i].key;
            float f = keymap[i].freq;
            if (GetAsyncKeyState(k) & 0x8000) {
                activate_voice(k, f);
            } else {
                deactivate_voice(k);
            }
        }

        // gerar buffer com mix de voices
        for (int i = 0; i < BUFFER_SIZE; i++) {
            double mix = 0.0;

            for (int v = 0; v < MAX_VOICES; v++) {
                if (voices[v].active && voices[v].freq > 0.0f) {
                    double phase = voices[v].phase; // 0..1
                    double sample = 0.0;

                    switch (play_type) {
                        case 0: // sine
                            sample = sin(2.0 * M_PI * phase);
                            break;
                        case 1: // square
                            sample = (sin(2.0 * M_PI * phase) > 0.0) ? 1.0 : -1.0;
                            break;
                        case 2: { // saw
                            // p ranges [0,1), map to [-1,1)
                            double p = phase;
                            sample = 2.0 * (p - floor(p + 0.5));
                        } break;
                        case 3: { // triangle
                            double p = phase;
                            // triangle wave from 0..1 -> -1..1
                            sample = 4.0 * fabs(p - 0.5) - 1.0;
                        } break;
                        case 4: { // square duty cycle
                            double p = phase;
                            sample = (fmod(p, 1.0) < duty_cycle) ? 1.0 : -1.0;
                        } break;
                        default:
                            sample = sin(2.0 * M_PI * phase);
                    }

                    // mix in
                    mix += sample;

                    // advance phase
                    voices[v].phase += voices[v].freq / (double)SAMPLE_RATE;
                    if (voices[v].phase >= 1.0) voices[v].phase -= floor(voices[v].phase);
                }
            }

            // normalize mix by active voices to avoid clipping (simple approach)
            int active_count = 0;
            for (int v = 0; v < MAX_VOICES; v++) if (voices[v].active) active_count++;
            if (active_count > 0) {
                mix /= (double)active_count; // simple per-voice normalization
            }

            // apply global volume (scale to -1..1)
            double vol_scale = (double)volume / 32767.0;
            mix *= vol_scale;

            mix = clamp_double(mix, -1.0, 1.0);
            buffer[i] = (short)(mix * 32767.0);
        }

        // prepara header e escreve
        WAVEHDR header;
        ZeroMemory(&header, sizeof(header));
        header.lpData = (LPSTR)buffer;
        header.dwBufferLength = BUFFER_SIZE * sizeof(short);
        header.dwFlags = 0;
        header.dwLoops = 0;

        if (waveOutPrepareHeader(hWave, &header, sizeof(header)) == MMSYSERR_NOERROR) {
            waveOutWrite(hWave, &header, sizeof(header));
            // espera terminar (bloqueia curto; evita acumular headers)
            while (!(header.dwFlags & WHDR_DONE)) {
                Sleep(1);
            }
            waveOutUnprepareHeader(hWave, &header, sizeof(header));
        } else {
            // se falhar, apenas continue (não fatal)
        }

        // pequeno sleep pra não consumir 100% CPU (já estamos gerando buffer)
        // Sleep aqui não necessário pois aguardamos WHDR_DONE, mas manter um tiny pause
        //Sleep(1);
    }

    waveOutReset(hWave);
    waveOutClose(hWave);
    return 0;
}
