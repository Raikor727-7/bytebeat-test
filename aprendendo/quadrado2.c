#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <math.h>

#define BUFFER_SIZE (44100 / 20)
#define SAMPLE_RATE 44100
#define PI 3.14159265358979323846

// Estrutura para armazenar informações de cada nota
typedef struct {
    int freq;
    int is_active;
    double phase;
} Note;

Note notes[] = {
    {'A', 0, 0.0},  // Dó - 261 Hz
    {'S', 0, 0.0},  // Ré - 293 Hz  
    {'D', 0, 0.0},  // Mi - 329 Hz
    {'F', 0, 0.0},  // Fá - 349 Hz
    {'G', 0, 0.0},  // Sol - 392 Hz
    {'H', 0, 0.0},  // Lá - 440 Hz
    {'J', 0, 0.0},  // Si - 493 Hz
    {'K', 0, 0.0}   // Dó alto - 523 Hz
};

int note_count = sizeof(notes) / sizeof(notes[0]);

int main() {
    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = 1;
    wfx.nSamplesPerSec = SAMPLE_RATE;
    wfx.wBitsPerSample = 16;
    wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    HWAVEOUT hWave;
    waveOutOpen(&hWave, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);

    short buffer[BUFFER_SIZE];
    
    // Frequências para cada tecla (escala musical)
    int frequencies[] = {261, 293, 329, 349, 392, 440, 493, 523};

    while (1) {
        if (GetAsyncKeyState(VK_ESCAPE)) break;

        // Verificar TODAS as teclas independentemente
        int active_notes = 0;
        for (int i = 0; i < note_count; i++) {
            if (GetAsyncKeyState(notes[i].freq)) {
                notes[i].is_active = 1;
                active_notes++;
            } else {
                notes[i].is_active = 0;
                notes[i].phase = 0.0;
            }
        }

        // Se há teclas pressionadas, gerar som misturado
        if (active_notes > 0) {
            // Gerar buffer de áudio misturando todas as notas ativas
            for (int i = 0; i < BUFFER_SIZE; i++) {
                double sample = 0.0;
                
                // Somar contribuição de cada nota ativa
                for (int n = 0; n < note_count; n++) {
                    if (notes[n].is_active) {
                        // Gerar onda senoidal para cada frequência
                        sample += sin(notes[n].phase * 2 * PI) * 0.3; // *0.3 para evitar clipping
                        notes[n].phase += frequencies[n] / (double)SAMPLE_RATE;
                        
                        // Manter a fase dentro de 0-2π
                        if (notes[n].phase >= 1.0) {
                            notes[n].phase -= 1.0;
                        }
                    }
                }
                
                // Normalizar e converter para 16-bit
                buffer[i] = (short)(sample * 16000);
            }

            waveOutReset(hWave);

            WAVEHDR header;
            header.lpData = (LPSTR)buffer;
            header.dwBufferLength = sizeof(buffer);
            header.dwFlags = 0;
            header.dwLoops = 0;

            waveOutPrepareHeader(hWave, &header, sizeof(WAVEHDR));
            waveOutWrite(hWave, &header, sizeof(WAVEHDR));
        } else {
            // Nenhuma tecla pressionada - pausa breve
            Sleep(10);
        }

        Sleep(2);
    }

    waveOutClose(hWave);
    return 0;
}