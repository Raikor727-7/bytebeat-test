#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <math.h>

// FUNÇÃO PARA SOM MAIS REALISTA (Piano-like)
double piano_wave(double time, double freq) {
    double sample = 0.0;
    
    // SÍNTESE ADITIVA - Múltiplos harmônicos como um piano real
    sample += 0.6 * sin(2 * 3.14159 * freq * time);                    // Fundamental
    sample += 0.3 * sin(2 * 3.14159 * freq * 2 * time);               // 2º harmônico
    sample += 0.2 * sin(2 * 3.14159 * freq * 3 * time);               // 3º harmônico  
    sample += 0.1 * sin(2 * 3.14159 * freq * 4 * time);               // 4º harmônico
    sample += 0.05 * sin(2 * 3.14159 * freq * 5 * time);              // 5º harmônico
    
    // ENVELOPE - Como um piano: ataque rápido, decay, sustain
    double envelope = 1.0;
    if (time < 0.01) envelope = time / 0.01;                          // Ataque
    else if (time < 0.3) envelope = 0.8 + 0.2 * exp(-time * 8);       // Decay
    else envelope = 0.7 * exp(-time * 2);                             // Release
    
    return sample * envelope;
}

// FUNÇÃO PARA BATIDA SIMPLES (acompanhamento)
double drum_beat(double time) {
    double beat = 0.0;
    
    // KICK nos tempos fortes (1 e 3)
    if (fmod(time, 0.5) < 0.05) {
        beat += 0.3 * sin(2 * 3.14159 * 80 * time) * exp(-time * 30);
    }
    
    // HIHAT nos tempos fracos
    if (fmod(time + 0.25, 0.5) < 0.02) {
        beat += 0.2 * ((rand() % 2000) - 1000) / 1000.0 * exp(-time * 50);
    }
    
    return beat;
}

int main() {
    #define BUFFER_SIZE 2205  // Buffer menor para resposta mais rápida

    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = 44100;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    HWAVEOUT hWave;
    waveOutOpen(&hWave, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);

    // NOTAS MUSICAIS (afinação mais precisa)
    #define C4 261.63
    #define D4 293.66
    #define E4 329.63
    #define F4 349.23
    #define G4 392.00
    #define A4 440.00
    #define B4 493.88
    #define C5 523.25

    // MELODIA: Twinkle Twinkle Little Star (arranjada)
    typedef struct {
        double freq;
        int duration;
        double volume;
    } Note;
    
    Note melody[] = {
        // Primeira frase - com acompanhamento
        {C4, 400, 0.8}, {C4, 400, 0.8}, {G4, 400, 0.8}, {G4, 400, 0.8}, 
        {A4, 400, 0.8}, {A4, 400, 0.8}, {G4, 800, 0.8},
        
        {F4, 400, 0.8}, {F4, 400, 0.8}, {E4, 400, 0.8}, {E4, 400, 0.8},
        {D4, 400, 0.8}, {D4, 400, 0.8}, {C4, 800, 0.8},
        
        // Segunda frase - mais suave
        {G4, 400, 0.7}, {G4, 400, 0.7}, {F4, 400, 0.7}, {F4, 400, 0.7},
        {E4, 400, 0.7}, {E4, 400, 0.7}, {D4, 800, 0.7},
        
        {G4, 400, 0.7}, {G4, 400, 0.7}, {F4, 400, 0.7}, {F4, 400, 0.7},
        {E4, 400, 0.7}, {E4, 400, 0.7}, {D4, 800, 0.7},
        
        // Reprise - com mais força
        {C4, 400, 0.9}, {C4, 400, 0.9}, {G4, 400, 0.9}, {G4, 400, 0.9},
        {A4, 400, 0.9}, {A4, 400, 0.9}, {G4, 800, 0.9},
        
        {F4, 400, 0.9}, {F4, 400, 0.9}, {E4, 400, 0.9}, {E4, 400, 0.9},
        {D4, 400, 0.9}, {D4, 400, 0.9}, {C4, 1200, 0.9}  // Final mais longo
    };

    int total_notes = sizeof(melody) / sizeof(melody[0]);

    printf("=== TWINKLE TWINKLE - COVER MELHORADO ===\n");
    printf("Pressione ESPACO para tocar\n");
    printf("Pressione ESC para sair\n");

    while (1) {
        if (GetAsyncKeyState(VK_ESCAPE)) break;

        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            printf("Tocando...\n");
            
            double global_time = 0.0;
            
            for (int note_idx = 0; note_idx < total_notes; note_idx++) {
                if (GetAsyncKeyState(VK_ESCAPE)) break;
                
                Note note = melody[note_idx];
                int samples_needed = (44100 * note.duration) / 1000;
                short* buffer = (short*)malloc(samples_needed * sizeof(short));
                
                // PRÉ-CALCULA batida para esta nota
                double* beat_samples = (double*)malloc(samples_needed * sizeof(double));
                for (int i = 0; i < samples_needed; i++) {
                    double t = global_time + (double)i / 44100.0;
                    beat_samples[i] = drum_beat(t);
                }
                
                for (int i = 0; i < samples_needed; i++) {
                    double t = (double)i / 44100.0;
                    
                    // MELODIA PRINCIPAL (piano)
                    double music_wave = piano_wave(t, note.freq) * note.volume;
                    
                    // BATIDA (ritmo)
                    double drum_wave = beat_samples[i] * 0.3;
                    
                    // MISTURA
                    double mixed = music_wave + drum_wave;
                    
                    // LIMITA PARA EVITAR DISTORÇÃO
                    if (mixed > 0.9) mixed = 0.9;
                    if (mixed < -0.9) mixed = -0.9;
                    
                    buffer[i] = (short)(30000 * mixed);
                }
                
                // TOCA A NOTA
                waveOutReset(hWave);
                WAVEHDR header;
                header.lpData = (LPSTR)buffer;
                header.dwBufferLength = samples_needed * sizeof(short);
                header.dwFlags = 0;
                header.dwLoops = 0;
                
                waveOutPrepareHeader(hWave, &header, sizeof(WAVEHDR));
                waveOutWrite(hWave, &header, sizeof(WAVEHDR));
                
                // ATUALIZA tempo global
                global_time += (double)note.duration / 1000.0;
                
                // ESPERA pela nota + libera memória
                Sleep(note.duration);
                free(buffer);
                free(beat_samples);
            }
            
            printf("Musica terminada! Pressione ESPACO para repetir\n");
        }
        
        Sleep(50);
    }

    waveOutClose(hWave);
    return 0;
}