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
        if (GetAsyncKeyState('A')) freq = 261.63; //DO
        else if (GetAsyncKeyState('S')) freq = 293.66; //RE
        else if (GetAsyncKeyState('D')) freq = 329.63; //MI
        else if (GetAsyncKeyState('F')) freq = 349.23; //FA
        else if (GetAsyncKeyState('G')) freq = 392.00; //SOL
        else if (GetAsyncKeyState('H')) freq = 440.00; //LA
        else if (GetAsyncKeyState('J')) freq = 493.88; //SI
        else if (GetAsyncKeyState('K')) freq = 523.25; //DO UMA OITAVA ACIMA
        else if (GetAsyncKeyState('Q')) freq = 293.66/2; //DO UMA OITAVA ABAIXO
        else if (GetAsyncKeyState('W')) freq = 329.63/2; //RE UMA OITAVA ABAIXO
        else if (GetAsyncKeyState('E')) freq = 349.23/2; //MI UMA OITAVA ABAIXO
        else if (GetAsyncKeyState('R')) freq = 392.00/2; //FA UMA OITAVA ABAIXO
        else if (GetAsyncKeyState('T')) freq = 440.00/2; //SOL UMA OITAVA ABAIXO
        else if (GetAsyncKeyState('Y')) freq = 493.88/2; //LA UMA OITAVA ABAIXO
        else if (GetAsyncKeyState('U')) freq = 293.66/2; //SI UMA OITAVA ABAIXO
        else if (GetAsyncKeyState('Z')) freq = 329.63*2; //RE UMA OITAVA ACIMA
        else if (GetAsyncKeyState('X')) freq = 349.23*2; //MI UMA OITAVA ACIMA
        else if (GetAsyncKeyState('C')) freq = 392.00*2; //FA UMA OITAVA ACIMA
        else if (GetAsyncKeyState('V')) freq = 440.00*2; //SOL UMA OITAVA ACIMA
        else if (GetAsyncKeyState('B')) freq = 493.88*2; //LA UMA OITAVA ACIMA
        else if (GetAsyncKeyState('N')) freq = 523.25*2; //SI UMA OITAVA ACIMA
        else if (GetAsyncKeyState('M')) freq = 523.25*3; //DO DUAS OITAVA ACIMA
        
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