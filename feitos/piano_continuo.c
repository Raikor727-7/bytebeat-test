#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <math.h>  // ⬅️ ADICIONAR ESTA LINHA

int main() {
    #define BUFFER_SIZE (44100 / 20)

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

    while (1) {
        int freq = 0;
        int play_chord = 0;  // ⬅️ CONTROLAR SE TOCA NORMAL OU "ACORDE"

        if (GetAsyncKeyState(VK_ESCAPE)) break;

        // notas DO a SI (A a K)
        if (GetAsyncKeyState('A')) freq = 261;
        else if (GetAsyncKeyState('S')) freq = 293;
        else if (GetAsyncKeyState('D')) freq = 329;
        else if (GetAsyncKeyState('F')) freq = 349;
        else if (GetAsyncKeyState('G')) freq = 392;
        else if (GetAsyncKeyState('H')) freq = 440;
        else if (GetAsyncKeyState('J')) freq = 493;
        else if (GetAsyncKeyState('K')) freq = 523;
        
        // ⬅️ SE APERTAR 'P', TOCA O "ACORDE" NA ÚLTIMA NOTA
        if (GetAsyncKeyState('P')) play_chord = 1;
        
        if (freq > 0) {
            short buffer[BUFFER_SIZE];
            int samples_per_period = 44100 / freq;

            for (int i = 0; i < BUFFER_SIZE; i++) {
                if (play_chord) {
                double t = (double)i / 44100.0;
                
                // FORMA DE ONDA MAIS RICA (quadrada suavizada)
                double wave1 = (i % (44100/freq) < (44100/freq)/2) ? 0.7 : -0.7;
                double wave2 = (i % (44100/(int)(freq*1.25)) < (44100/(int)(freq*1.25))/2) ? 0.5 : -0.5;
                double wave3 = (i % (44100/(int)(freq*1.5)) < (44100/(int)(freq*1.5))/2) ? 0.4 : -0.4;
                
                // Mistura as três ondas quadradas (mais harmônicos naturais)
                double mixed = wave1 + wave2 + wave3;
                
                buffer[i] = (short)(15000 * mixed);
            } else {
                    // ⬅️ MODO NORMAL - ONDA QUADRADA ORIGINAL
                    buffer[i] = (i % samples_per_period < samples_per_period / 2) ? 20000 : -20000;
                }
            }

            waveOutReset(hWave);

            WAVEHDR header;
            header.lpData = (LPSTR)buffer;
            header.dwBufferLength = sizeof(buffer);
            header.dwFlags = 0;
            header.dwLoops = 0;

            waveOutPrepareHeader(hWave, &header, sizeof(WAVEHDR));
            waveOutWrite(hWave, &header, sizeof(WAVEHDR));
        }

        Sleep(2);
    }

    waveOutClose(hWave);
    return 0;
}