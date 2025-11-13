#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

#define PI 3.14159265358979323846

// ==================== SISTEMA DE REVERB ====================
typedef struct {
    double *buffer;
    int size;
    int pos;
    double decay;
} ReverbDelay;

ReverbDelay* create_reverb(int size_ms, double decay) {
    ReverbDelay* rd = (ReverbDelay*)malloc(sizeof(ReverbDelay));
    rd->size = (44100 * size_ms) / 1000;
    rd->buffer = (double*)calloc(rd->size, sizeof(double));
    rd->pos = 0;
    rd->decay = decay;
    return rd;
}

double process_reverb(ReverbDelay* rd, double input) {
    double output = rd->buffer[rd->pos];
    rd->buffer[rd->pos] = input + output * rd->decay;
    rd->pos = (rd->pos + 1) % rd->size;
    return output;
}

// ==================== SISTEMA DE CHORUS ====================
typedef struct {
    double phase;
    double rate;
    double depth;
    int delay_size;
    double *delay_buffer;
    int delay_pos;
} Chorus;

Chorus* create_chorus(double rate, double depth, int delay_ms) {
    Chorus* c = (Chorus*)malloc(sizeof(Chorus));
    c->phase = 0.0;
    c->rate = rate;
    c->depth = depth;
    c->delay_size = (44100 * delay_ms) / 1000;
    c->delay_buffer = (double*)calloc(c->delay_size, sizeof(double));
    c->delay_pos = 0;
    return c;
}

double process_chorus(Chorus* c, double input) {
    // LFO para modulação
    double lfo = sin(c->phase * 2 * PI);
    c->phase += c->rate / 44100.0;
    if (c->phase >= 1.0) c->phase -= 1.0;
    
    // Calcula delay variável
    int delay_samples = (int)(c->delay_size / 2 * (1.0 + lfo * c->depth));
    int read_pos = (c->delay_pos - delay_samples + c->delay_size) % c->delay_size;
    
    // Escreve no buffer
    c->delay_buffer[c->delay_pos] = input;
    c->delay_pos = (c->delay_pos + 1) % c->delay_size;
    
    // Lê do buffer com delay modulado
    double delayed = c->delay_buffer[read_pos];
    
    // Mixa sinal original com delayed
    return 0.7 * input + 0.3 * delayed;
}

// ==================== SÍNTESE AVANÇADA ====================
double advanced_piano_wave(double time, double freq, double velocity) {
    double sample = 0.0;
    
    // OSCILADORES MÚLTIPLOS com fases ligeiramente diferentes
    sample += 0.6 * sin(2 * PI * freq * time);                    // Principal
    sample += 0.4 * sin(2 * PI * freq * 1.002 * time + 0.1);     // Detune sutil
    sample += 0.3 * sin(2 * PI * freq * 2 * time);               // 2º harmônico
    sample += 0.2 * sin(2 * PI * freq * 3 * time + 0.2);         // 3º harmônico
    sample += 0.15 * sin(2 * PI * freq * 4 * time);              // 4º harmônico
    sample += 0.1 * sin(2 * PI * freq * 5 * time + 0.3);         // 5º harmônico
    
    // ENVELOPE AVANÇADO (ADSR realístico)
    double envelope = 0.0;
    if (time < 0.005) {
        envelope = time / 0.005;                                 // Attack rápido
    } else if (time < 0.1) {
        envelope = 0.8 + 0.2 * exp(-(time - 0.005) * 30);        // Decay
    } else if (time < 0.5) {
        envelope = 0.7 * exp(-(time - 0.1) * 4);                 // Sustain
    } else {
        envelope = 0.5 * exp(-(time - 0.5) * 6);                 // Release
    }
    
    // FILTRO PASA-BAIXA DINÂMICO (simula corpo do piano)
    static double filter_mem = 0.0;
    double cutoff = 0.1 + 0.3 * exp(-time * 4);                  // Cutoff decai com tempo
    double filtered = filter_mem + cutoff * (sample - filter_mem);
    filter_mem = filtered;
    
    return filtered * envelope * velocity;
}

// ==================== BATERIA AVANÇADA ====================
double advanced_drum_beat(double time) {
    double beat = 0.0;
    double beat_time = fmod(time, 1.0);  // Ciclo de 1 segundo
    
    // KICK - mais punchy
    if (beat_time < 0.05 || (beat_time > 0.5 && beat_time < 0.55)) {
        double kick_env = exp(-fmod(time, 0.5) * 40);
        beat += 0.4 * sin(2 * PI * 60 * time) * kick_env;
        beat += 0.2 * sin(2 * PI * 80 * time * 2) * kick_env;
    }
    
    // SNARE - nos tempos 2 e 4
    if ((beat_time > 0.25 && beat_time < 0.28) || (beat_time > 0.75 && beat_time < 0.78)) {
        double snare_env = exp(-(fmod(time + 0.25, 0.5)) * 50);
        for(int i = 0; i < 5; i++) {
            beat += 0.15 * ((rand() % 2000) - 1000) / 1000.0 * snare_env;
        }
        beat += 0.1 * sin(2 * PI * 180 * time) * snare_env;
    }
    
    // HI-HAT - padrão constante
    if (fmod(beat_time * 4, 1.0) < 0.1) {
        double hat_env = exp(-fmod(time * 8, 1.0) * 80);
        for(int i = 0; i < 3; i++) {
            beat += 0.08 * ((rand() % 2000) - 1000) / 1000.0 * hat_env;
        }
    }
    
    return beat;
}

// ==================== BAIXO ====================
double bass_line(double time, double freq) {
    // Baixo mais suave com portamento
    static double last_freq = 0.0;
    if (last_freq == 0.0) last_freq = freq;
    
    // Suaviza transições de frequência
    double smooth_freq = last_freq + 0.1 * (freq - last_freq);
    last_freq = smooth_freq;
    
    double bass = 0.5 * sin(2 * PI * smooth_freq * 0.5 * time);  // Uma oitava abaixo
    bass += 0.3 * sin(2 * PI * smooth_freq * 0.5 * 2 * time);    // Harmônico
    
    // Envelope do baixo
    double env = exp(-fmod(time, 2.0) * 3);
    return bass * env * 0.4;
}

// ==================== ACORDES ====================
double chord_pad(double time, double root_freq) {
    double chord = 0.0;
    
    // Acorde maior (root, third, fifth)
    double frequencies[] = {
        root_freq * 0.5,        // Root uma oitava abaixo
        root_freq * 0.5 * 1.25, // Third
        root_freq * 0.5 * 1.5   // Fifth
    };
    
    for(int i = 0; i < 3; i++) {
        chord += 0.2 * sin(2 * PI * frequencies[i] * time);
        chord += 0.1 * sin(2 * PI * frequencies[i] * 2 * time); // Harmônicos
    }
    
    // Envelope longo e suave
    double env = 0.3 + 0.7 * exp(-time * 0.5);
    return chord * env * 0.3;
}

int main() {
    srand((unsigned int)time(NULL));
    
    #define BUFFER_SIZE 4410  // 100ms buffer

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

    // INICIALIZA EFEITOS
    ReverbDelay* reverb1 = create_reverb(800, 0.6);   // Reverb longo
    ReverbDelay* reverb2 = create_reverb(300, 0.4);   // Reverb curto
    Chorus* chorus = create_chorus(0.5, 0.3, 20);     // Chorus suave

    // NOTAS MUSICAIS
    #define C4 261.63
    #define D4 293.66
    #define E4 329.63
    #define F4 349.23
    #define G4 392.00
    #define A4 440.00
    #define B4 493.88
    #define C5 523.25

    // ARRANJO COMPLETO - TWINKLE TWINKLE ÉPICO
    typedef struct {
        double melody_freq;
        double bass_freq;
        double chord_freq;
        int duration;
        double velocity;
    } ArrangementNote;
    
    ArrangementNote epic_arrangement[] = {
        // INTRO - Só melodia suave
        {C4, C4, 0, 400, 0.6}, {C4, C4, 0, 400, 0.6}, 
        {G4, G4, 0, 400, 0.7}, {G4, G4, 0, 400, 0.7},
        {A4, A4, 0, 400, 0.8}, {A4, A4, 0, 400, 0.8}, {G4, G4, 0, 800, 0.8},
        
        // VERSO 1 - Entra o baixo
        {F4, F4, F4, 400, 0.8}, {F4, F4, F4, 400, 0.8}, 
        {E4, E4, C4, 400, 0.8}, {E4, E4, C4, 400, 0.8},
        {D4, D4, G4, 400, 0.8}, {D4, D4, G4, 400, 0.8}, {C4, C4, C4, 800, 0.8},
        
        // VERSO 2 - Entram acordes completos
        {G4, G4, G4, 400, 0.9}, {G4, G4, G4, 400, 0.9},
        {F4, F4, F4, 400, 0.9}, {F4, F4, F4, 400, 0.9},
        {E4, E4, C4, 400, 0.9}, {E4, E4, C4, 400, 0.9}, {D4, D4, G4, 800, 0.9},
        
        // CLÍMAX - Todos os elementos
        {G4, G4, G4, 400, 1.0}, {G4, G4, G4, 400, 1.0},
        {F4, F4, F4, 400, 1.0}, {F4, F4, F4, 400, 1.0},
        {E4, E4, C4, 400, 1.0}, {E4, E4, C4, 400, 1.0}, {D4, D4, G4, 800, 1.0},
        
        // REPRISE FINAL
        {C4, C4, C4, 400, 0.9}, {C4, C4, C4, 400, 0.9},
        {G4, G4, G4, 400, 0.9}, {G4, G4, G4, 400, 0.9},
        {A4, A4, A4, 400, 0.9}, {A4, A4, A4, 400, 0.9}, {G4, G4, G4, 800, 0.9},
        
        {F4, F4, F4, 400, 0.8}, {F4, F4, F4, 400, 0.8},
        {E4, E4, C4, 400, 0.8}, {E4, E4, C4, 400, 0.8},
        {D4, D4, G4, 400, 0.8}, {D4, D4, G4, 400, 0.8}, {C4, C4, C4, 1600, 0.7},
        
        // OUTRO - Fade out
        {C4, C4, C4, 800, 0.5}, {G4, G4, G4, 800, 0.4}, 
        {C5, C5, C5, 1600, 0.3}, {0, 0, 0, 800, 0.0}  // Fade final
    };

    int total_sections = sizeof(epic_arrangement) / sizeof(epic_arrangement[0]);

    printf("=== TWINKLE TWINKLE - ULTIMATE EDITION ===\n");
    printf("Produzido por: Seu Sintetizador Épico\\n");
    printf("Pressione ESPACO para o SHOW\\n");
    printf("Pressione ESC para sair\\n");

    while (1) {
        if (GetAsyncKeyState(VK_ESCAPE)) break;

        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            printf("Iniciando performance épica...\\n");
            
            double global_time = 0.0;
            
            for (int section = 0; section < total_sections; section++) {
                if (GetAsyncKeyState(VK_ESCAPE)) break;
                
                ArrangementNote note = epic_arrangement[section];
                int samples_needed = (44100 * note.duration) / 1000;
                short* buffer = (short*)malloc(samples_needed * sizeof(short));
                
                for (int i = 0; i < samples_needed; i++) {
                    double t = global_time + (double)i / 44100.0;
                    double sample = 0.0;
                    
                    // MELODIA PRINCIPAL (piano avançado)
                    if (note.melody_freq > 0) {
                        double melody = advanced_piano_wave(t, note.melody_freq, note.velocity);
                        sample += melody;
                    }
                    
                    // LINHA DE BAIXO
                    if (note.bass_freq > 0) {
                        sample += bass_line(t, note.bass_freq);
                    }
                    
                    // ACORDES DE FUNDO
                    if (note.chord_freq > 0) {
                        sample += chord_pad(t, note.chord_freq);
                    }
                    
                    // BATIDA RÍTMICA
                    sample += advanced_drum_beat(t) * 0.4;
                    
                    // APLICA EFEITOS EM CADEIA
                    sample = process_chorus(chorus, sample);
                    sample = process_reverb(reverb1, sample);
                    sample = process_reverb(reverb2, sample);
                    
                    // LIMITAÇÃO PROFISSIONAL
                    if (sample > 0.95) sample = 0.95;
                    if (sample < -0.95) sample = -0.95;
                    
                    buffer[i] = (short)(32000 * sample);
                }
                
                // PERFORMANCE
                waveOutReset(hWave);
                WAVEHDR header;
                header.lpData = (LPSTR)buffer;
                header.dwBufferLength = samples_needed * sizeof(short);
                header.dwFlags = 0;
                header.dwLoops = 0;
                
                waveOutPrepareHeader(hWave, &header, sizeof(WAVEHDR));
                waveOutWrite(hWave, &header, sizeof(WAVEHDR));
                
                global_time += (double)note.duration / 1000.0;
                Sleep(note.duration);
                free(buffer);
                
                // FEEDBACK VISUAL
                printf("Progresso: %d/%d\\r", section + 1, total_sections);
            }
            
            printf("\\n🎉 PERFORMANCE CONCLUÍDA! Obra-prima finalizada! 🎉\\n");
        }
        
        Sleep(50);
    }

    // LIMPEZA
    free(reverb1->buffer);
    free(reverb1);
    free(reverb2->buffer);
    free(reverb2);
    free(chorus->delay_buffer);
    free(chorus);
    
    waveOutClose(hWave);
    return 0;
}