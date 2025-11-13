#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <math.h>  // ⬅️ ADICIONAR ESTA LINHA

int main(){
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
        float freq = 0;
        int play_chord = 0;  // ⬅️ CONTROLAR SE TOCA NORMAL OU "ACORDE"

        if (GetAsyncKeyState(VK_ESCAPE)) break;

        // notas DO a SI (A a K)
        if (GetAsyncKeyState('A')) freq = 261.63;
        else if (GetAsyncKeyState('S')) freq = 293.66;
        else if (GetAsyncKeyState('D')) freq = 329.63;
        else if (GetAsyncKeyState('F')) freq = 349.23;
        else if (GetAsyncKeyState('G')) freq = 392.00;
        else if (GetAsyncKeyState('H')) freq = 440.00;
        else if (GetAsyncKeyState('J')) freq = 493.88;
        else if (GetAsyncKeyState('K')) freq = 523.25;
        
        if (freq > 0) {
            short buffer[BUFFER_SIZE];
            int samples_per_period = 44100 / freq;

            for (int i = 0; i < BUFFER_SIZE; i++) {
                double t = (double)i / 44100.0;
                buffer[i] = (short)(30000 * sin(2.0 * 3.14159265 * freq * t));
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