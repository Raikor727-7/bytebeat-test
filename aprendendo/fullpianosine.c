#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <math.h>  // ⬅️ ADICIONAR ESTA LINHA
#include <conio.h>  

int main(){
    #define BUFFER_SIZE (44100 / 20)
    #define MAX_VOICES 5

    typedef struct {
        float freq;
        int active;
        double phase;
    } Voice;

    Voice voices[MAX_VOICES] = {0};

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

    int play_type = 0;  // ⬅️ CONTROLAR TIPO
    int volume = 28000;
    float duty_cycle = 0.3;

    while (1) {
        float freq = 0;
        
        if (_kbhit()){
            char tecla = _getch();

            

            // switch(tecla){
            //     case '[':
            //         volume += 500;
            //         if (volume >= 32767){
            //             printf("volume maximo");
            //             volume = 32767;
            //         }
            //         printf("Volume: %d\r", volume);
            //         break;
            //     case ']':
            //         volume -= 500;
            //         if (volume <= 10000){
            //             printf("volume minimo");
            //             volume = 10000;
            //         }
            //         printf("Volume: %d\r", volume);
            //         break;
            // }
            switch (tecla)
            {
            case '1':
                system("cls");
                printf("modo sine\n");
                break;
            case '2':
                system("cls");
                printf("modo quadrado\n");
                play_type = 1;
                break;
            case '3':
                system("cls");
                printf("modo serra\n");
                play_type = 2;
                break;
            case '4':
                system("cls");
                printf("modo triangle\n");
                play_type = 3;
                break;
            case '5':
                system("cls");
                printf("modo quadrado com duty (<- e -> para modificar)\n");
                play_type = 4;
                break;
            }
            
        }

        if (GetAsyncKeyState(VK_UP)){
                volume += 500;
                if (volume >= 32767){
                    volume = 32767;
                    printf("\rVolume: %d    volume maximo", volume);
                }
                else{
                    printf("\rVolume: %d                        ", volume); 
                }
                Sleep(2);      
            }
        if (GetAsyncKeyState(VK_DOWN)){
                volume -= 500;
                if (volume <= 5000){
                    volume = 5000;
                    printf("\rVolume: %d    volume minimo", volume);
                }
                else{
                    printf("\rVolume: %d                        ", volume); 
                }
                Sleep(2);
        }
        if (GetAsyncKeyState(VK_RIGHT)){
                duty_cycle += 0.1;
                if (duty_cycle >= 1){
                    duty_cycle = 1;
                    printf("\rduty cycle: %f    max", duty_cycle);
                }
                else{
                    printf("\rduty cycle: %f          ", duty_cycle); 
                    Sleep(2000);
                }
        }
        if (GetAsyncKeyState(VK_LEFT)){
                duty_cycle -= 0.1;
                if (duty_cycle <= 0){
                    duty_cycle = 0;
                    printf("\rduty cycle: %f    min", duty_cycle);
                }
                else{
                    printf("\rduty cycle: %f          ", duty_cycle); 
                    Sleep(2000);
                }                
        }

        if (GetAsyncKeyState(VK_ESCAPE)) break;

        // notas 
        if (GetAsyncKeyState('A')){
            freq = 261.63; //DO                 
        }
        if (GetAsyncKeyState('S')){
            freq = 293.66; //RE              
        }
        if (GetAsyncKeyState('D')){
            freq = 329.63; //MI            
        }
        if (GetAsyncKeyState('F')){
            freq = 349.23; //FA                 
        }
        if (GetAsyncKeyState('G')){
            freq = 392.00; //SOL     
        }
        if (GetAsyncKeyState('H')){
            freq = 440.00; //LA             
        }
        if (GetAsyncKeyState('J')){
            freq = 493.88; //SI                  
        }
        if (GetAsyncKeyState('K')){
            freq = 261.63 * 2;//DO UMA OITAVA ACIMA
        }
        if (GetAsyncKeyState('Q')){
            freq = 261.63/2;//DO UMA OITAVA ABAIXO
        } 
        if (GetAsyncKeyState('W')){
            freq = 293.66/2;//RE UMA OITAVA ABAIXO
        } 
        if (GetAsyncKeyState('E')){
            freq = 329.63; //MI UMA OITAVA ABAIXO
        }
        if (GetAsyncKeyState('R')){
            freq = 349.23/2;//FA UMA OITAVA ABAIXO
        }
        if (GetAsyncKeyState('T')){
            freq = 392.00/2;//SOL UMA OITAVA ABAIXO 
        }
        if (GetAsyncKeyState('Y')){
            freq = 440.00/2;//LA UMA OITAVA ABAIXO   
        }
        if (GetAsyncKeyState('U')){
            freq = 493.88/2;//SI UMA OITAVA ABAIXO   
        }
        if (GetAsyncKeyState('Z')){
            freq = 293.66*2;//RE UMA OITAVA ACIMA
        }
        if (GetAsyncKeyState('X')){
            freq = 329.63*2;//MI UMA OITAVA ACIMA
        }
        if (GetAsyncKeyState('C')){
            freq = 349.23*2; //FA UMA OITAVA ACIMA
        }
        if (GetAsyncKeyState('V')){
            freq = 392.00*2;//SOL UMA OITAVA ACIMA
        }
        if (GetAsyncKeyState('B')){
            freq = 440.00*2; //LA UMA OITAVA ACIMA
        }
        if (GetAsyncKeyState('N')){
            freq = 493.88*2; //SI UMA OITAVA ACIMA
        } 
        if (GetAsyncKeyState('M')){
            freq = 261.63 * 3; //DO DUAS OITAVA ACIMA
        }
        
        if (freq > 0) {
            short buffer[BUFFER_SIZE];
            int samples_per_period = 44100 / freq;

            if(play_type == 0){
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    double t = (double)i / 44100.0;
                    buffer[i] = (short)(volume * sin(2.0 * 3.14159265 * freq * t));
                }
            } else if (play_type == 1){
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    int samples_per_period = 44100 / freq;
                    buffer[i] = (i % samples_per_period < samples_per_period / 2) ? volume : -volume;
                }
            }else if (play_type == 2){
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    double t = (double)i / (44100.0 / freq);
                    double saw = 2.0 * (t - floor(t + 0.5));
                    buffer[i] = (short)(volume * saw);
                }
            }else if (play_type == 3){
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    double t = fmod((double)i / (44100.0 / freq), 1.0);
                    double tri = 4.0 * fabs(t - 0.5) - 1.0;
                    buffer[i] = (short)(volume * tri);
                }
            }
            else if (play_type == 4){
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    int samples_per_period = 44100 / freq;
                    int threshold = (int)(samples_per_period * duty_cycle);
                    buffer[i] = (i % samples_per_period < threshold) ? volume : -volume;
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