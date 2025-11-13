#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <math.h>

int main() {
    #define BUFFER_SIZE (44100 / 20) // menor buffer = resposta mais rápida

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
        else freq = 0;

        if (freq > 0) {
            short buffer[BUFFER_SIZE];
            int samples_per_period = 44100 / freq;

            // onda quadrada simples
            for (int i = 0; i < BUFFER_SIZE; i++) {
                double t = fmod((double)i / samples_per_period, 1.0);
                double tri = 4.0 * fabs(t - 0.5) - 1.0;
                buffer[i] = (short)(30000 * tri);
            }

            // reseta áudio anterior pra não acumular buffers
            waveOutReset(hWave);

            WAVEHDR header;
            header.lpData = (LPSTR)buffer;
            header.dwBufferLength = sizeof(buffer);
            header.dwFlags = 0;
            header.dwLoops = 0;

            waveOutPrepareHeader(hWave, &header, sizeof(WAVEHDR));
            waveOutWrite(hWave, &header, sizeof(WAVEHDR));
        }

        Sleep(2); // tempo curto = resposta imediata
    }

    waveOutClose(hWave);
    return 0;
}
