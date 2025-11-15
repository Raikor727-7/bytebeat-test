#include <stdio.h>
#include <windows.h>
#include <mmsystem.h>
#include <conio.h>
#include <math.h>

int main() {
    #ifndef M_PI
    #define M_PI 3.14159265358979323846
    #endif

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

    printf("Bateria digital: g=bumbo, f=snare, h=hihat, t=tom, ESC=sair\n");

    while (1) {
        if (_kbhit()) {
            char tecla = _getch();
            if (tecla == 27) break;

            int freq = 0;
            double decay = 0.001;
            double duration = 0.2; // segundos
            int altura = 30000;

            switch (tecla) {
                case 'g': freq = 60; decay = 0.0008; duration = 0.3; altura = 30000; break;
                case 'r': freq = 6000; decay = 0.002; duration = 0.4; break;
                case 'f': freq = 180; decay = 0.005; duration = 0.15; altura = 60000; break;
                case 'y': freq = 3000; decay = 0.0008; duration = 0.6; break;
                case 'h': freq = 10000; decay = 0.02; duration = 0.1; altura = 300000; break;
                case 't': freq = 200; decay = 0.0015; duration = 0.25; altura = 60000; break;
                default: continue;
            }

            int totalSamples = (int)(44100 * duration);
            short* buffer = (short*)malloc(totalSamples * sizeof(short));

            for (int i = 0; i < totalSamples; i++) {
                double amp;
                if (tecla == 'h' || tecla == 'r' || tecla =='y')
                    amp = ((rand() % 2000) - 1000) / 1000.0 * exp(-decay * i);
                //else if(tecla == 'r' || tecla == 'y'){
                //    amp = sin(2 * M_PI * freq * i + sin(2 * M_PI * 20 * i)) * exp(-decay * i);
                //}
                else
                    amp = sin(2 * M_PI * freq * i / 44100) * exp(-decay * i);

                buffer[i] = (short)(amp * altura);
            }

            WAVEHDR header = {0};
            header.lpData = (LPSTR)buffer;
            header.dwBufferLength = totalSamples * sizeof(short);

            waveOutPrepareHeader(hWave, &header, sizeof(WAVEHDR));
            waveOutWrite(hWave, &header, sizeof(WAVEHDR));

            while (waveOutUnprepareHeader(hWave, &header, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING)
                Sleep(1);

            free(buffer);
        }
    }

    waveOutClose(hWave);
    return 0;
}
