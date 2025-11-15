#include <stdio.h>      // entrada e saída básica (printf, etc.)
#include <windows.h>    // para WAVEFORMATEX, HWAVEOUT, Sleep, etc.
#include <mmsystem.h>   // para waveOutOpen, waveOutWrite, etc.
#include <conio.h>      // para _kbhit() e _getch() (leitura de teclado em tempo real)

int main(){
    #define BUFFER_SIZE (44100 / 20)

    WAVEFORMATEX wfx;
    wfx.wFormatTag = WAVE_FORMAT_PCM;   // som simples (PCM)
    wfx.nChannels = 1;                  // 1 = mono
    wfx.nSamplesPerSec = 44100;          // 8000 amostras por segundo
    wfx.wBitsPerSample = 16;             // cada amostra = 1 byte
    wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    HWAVEOUT hWave;
    waveOutOpen(&hWave, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL);

    while(1){
        int notas[] = {261, 293, 329, 349, 392, 440, 493, 523};
        int freq = 0;

        if(_kbhit()){
            char tecla = _getch();

            if(tecla == 27) break;

            switch (tecla){
                case 'a': freq = 261; break;
                case 's': freq = 293; break;
                case 'd': freq = 329; break;
                case 'f': freq = 349; break;
                case 'g': freq = 392; break;
                case 'h': freq = 440; break;
                case 'j': freq = 493; break;
                case 'k': freq = 523; break; 
                default: freq = 0; break;
            }
            short buffer[BUFFER_SIZE];
            int samples_per_period = 44100 / freq;

            for (int i = 0; i < BUFFER_SIZE; i++) {
                buffer[i] = (i % samples_per_period < samples_per_period / 2) ? 30000 : -30000;
            }
            while(freq > 0){
                WAVEHDR header;
                header.lpData = (LPSTR)buffer;
                header.dwBufferLength = BUFFER_SIZE;
                header.dwFlags = 0;
                header.dwLoops = 0;
                waveOutPrepareHeader(hWave, &header, sizeof(WAVEHDR));
                waveOutWrite(hWave, &header, sizeof(WAVEHDR));
                Sleep(BUFFER_SIZE * 1000 / 44100);
                freq = 0;
                }    
            }     
            
        }
        
}
