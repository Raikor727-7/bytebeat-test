#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <math.h>

#define SAMPLE_RATE 44100
#define BUFFER_SIZE (SAMPLE_RATE / 20) // 50ms buffer
#define NYQUIST (SAMPLE_RATE / 2)
#define M_PIN 3.1415926535897323
#define MAX_HARMONICS 10 // Limitar harmônicos para evitar aliasing
#define TWO_PI (2.0 * M_PIN)

// Estrutura para geração de onda com phase increment
typedef struct {
    double phase;
    double phase_inc;
    int active;
} Voice;

Voice voices[8]; // Uma voz para cada tecla
int frequencies[] = {261, 293, 329, 349, 392, 440, 493, 523};
char keys[] = {'A', 'S', 'D', 'F', 'G', 'H', 'J', 'K'};

// Gerar onda quadrada com harmônicos limitados (band-limited)
double band_limited_square(double phase, double freq) {
    double sample = 0.0;
    
    // Série de Fourier para onda quadrada: 4/π * Σ(sin(2π(2k-1)ft)/(2k-1))
    for (int k = 1; k <= MAX_HARMONICS; k++) {
        int harmonic = 2 * k - 1; // Harmônicos ímpares
        double harmonic_freq = freq * harmonic;
        
        // Verificar se o harmônico está abaixo de Nyquist
        if (harmonic_freq < NYQUIST) {
            double amplitude = 4.0 / (M_PIN * harmonic);
            sample += amplitude * sin(TWO_PI * harmonic * phase);
        }
    }
    
    return sample;
}

// Gerar onda serra com harmônicos limitados
double band_limited_saw(double phase, double freq) {
    double sample = 0.0;
    
    // Série de Fourier para onda serra: 1/2 - 1/π * Σ(sin(2πkft)/k)
    for (int k = 1; k <= MAX_HARMONICS; k++) {
        double harmonic_freq = freq * k;
        
        // Verificar se o harmônico está abaixo de Nyquist
        if (harmonic_freq < NYQUIST) {
            double amplitude = -2.0 / (M_PIN * k);
            sample += amplitude * sin(TWO_PI * k * phase);
        }
    }
    
    return sample + 0.5; // Offset de DC para onda serra
}

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

    // Inicializar vozes
    for (int i = 0; i < 8; i++) {
        voices[i].phase = 0.0;
        voices[i].phase_inc = TWO_PI * frequencies[i] / SAMPLE_RATE;
        voices[i].active = 0;
    }

    while (1) {
        if (GetAsyncKeyState(VK_ESCAPE)) break;

        // Verificar TODAS as teclas independentemente para polifonia
        int active_voices = 0;
        for (int i = 0; i < 8; i++) {
            if (GetAsyncKeyState(keys[i])) {
                voices[i].active = 1;
                active_voices++;
            } else {
                voices[i].active = 0;
            }
        }

        if (active_voices > 0) {
            short buffer[BUFFER_SIZE];
            
            // Gerar áudio misturando todas as vozes ativas
            for (int i = 0; i < BUFFER_SIZE; i++) {
                double mixed_sample = 0.0;
                
                // Somar contribuição de cada voz ativa
                for (int v = 0; v < 8; v++) {
                    if (voices[v].active) {
                        // Escolha o tipo de onda (mude para band_limited_saw se quiser serra)
                        double voice_sample = band_limited_square(voices[v].phase, frequencies[v]);
                        
                        // Normalizar amplitude baseado no número de vozes ativas
                        voice_sample *= 0.7 / active_voices; // Evitar clipping
                        mixed_sample += voice_sample;
                        
                        // Incrementar fase (MÉTODO CORRETO)
                        voices[v].phase += voices[v].phase_inc;
                        if (voices[v].phase >= TWO_PI) {
                            voices[v].phase -= TWO_PI;
                        }
                    }
                }
                
                // Converter para 16-bit com clamping
                int sample_16bit = (int)(mixed_sample * 30000);
                
                // Clamping para evitar saturação
                if (sample_16bit > 32767) sample_16bit = 32767;
                if (sample_16bit < -32768) sample_16bit = -32768;
                
                buffer[i] = (short)sample_16bit;
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
            Sleep(10); // Pausa quando não há teclas pressionadas
        }

        Sleep(2);
    }

    waveOutClose(hWave);
    return 0;
}