#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <math.h>  // ⬅️ ADICIONAR ESTA LINHA
#include <conio.h>  

#define BUFFER_SIZE (44100 / 20)
#define MAX_VOICES 5
#define PI 3.14159265358979323

typedef struct {
    float freq;
    int active;
    double phase;
    int key;
} Voice;

Voice voices[MAX_VOICES] = {0};

void activate_voice(int key, float freq) {
    // PROCURE uma voice que esteja com active = 0
    for (int v = 0; v < MAX_VOICES; v++){
        if(!voices[v].active){
            voices[v].freq = freq;
            voices[v].active = 1;
            for (int i = 0; i < BUFFER_SIZE; i++) {
                voices[v].phase = (double)i / 44100.0;
            }
            voices[v].key = key;
            return;
        }
    }
}
void deactivate_voice(int key) {
    // PROCURE uma voice que esteja com active = 0
    for (int v = 0; v < MAX_VOICES; v++){
        if(voices[v].active && voices[v].key == key){
            voices[v].freq = 0;
            voices[v].active = 0;
            return;
        }
    }
}


int main(){
   

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

    //notas
    typedef struct {
        int key;
        float freq;
    } KeyMap;

    KeyMap keymap[] = {
        {'A', 261.63},   // DO
        {'S', 293.66},   // RE
        {'D', 329.63},   // MI
        {'F', 349.23},   // FA
        {'G', 392.00},   // SOL
        {'H', 440.00},   // LA
        {'J', 493.88},   // SI
        {'K', 261.63*2}, // DO ↑
        {'Q', 261.63/2}, // DO ↓
        {'W', 293.66/2}, // RE ↓
        {'E', 329.63/2}, // MI ↓
        {'R', 349.23/2}, // FA ↓
        {'T', 392.00/2}, // SOL ↓
        {'Y', 440.00/2}, // LA ↓
        {'U', 493.88/2}, // SI ↓
        {'Z', 293.66*2}, // RE ↑
        {'X', 329.63*2}, // MI ↑
        {'C', 349.23*2}, // FA ↑
        {'V', 392.00*2}, // SOL ↑
        {'B', 440.00*2}, // LA ↑
        {'N', 493.88*2}, // SI ↑
        {'M', 261.63*3}, // DO ↑↑
    };
    int KEYMAP_SIZE = sizeof(keymap) / sizeof(KeyMap);

    while (1) {
        float freq = 0;
        
        if (_kbhit()){
            char tecla = _getch();

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
        // Procurar notas na tabela
        freq = 0;
        for (int i = 0; i < KEYMAP_SIZE; i++) {
            int k = keymap[i].key;
            float f = keymap[i].freq;
            
            if (GetAsyncKeyState(k) & 0x8000) {
                freq = keymap[i].freq;
                activate_voice(k, f);
                break;
            }else{
                deactivate_voice(k);
            }
        }

        short buffer[BUFFER_SIZE];

        for (int i = 0; i < BUFFER_SIZE; i++) {
            double mix = 0;

            for (int v = 0; v < MAX_VOICES; v++) {
                if (voices[v].active) {

                    double sample = 0;
                    double p = voices[v].phase;

                    switch (play_type) {
                        case 0: // seno
                            sample = sin(2 * PI * p);
                            break;

                        case 1: // quadrada
                            sample = (sin(2 * PI * p) > 0 ? 1 : -1);
                            break;

                        case 2: // serra
                            sample = 2 * (p - floor(p + 0.5));
                            break;

                        case 3: // triangular
                            sample = 2 * fabs(2 * (p - floor(p + 0.5))) - 1;
                            break;

                        case 4: // square duty
                            sample = (fmod(p, 1.0) < duty_cycle) ? 1 : -1;
                            break;
                    }

                    mix += sample;

                    voices[v].phase += voices[v].freq / 44100.0;
                    if (voices[v].phase >= 1.0) voices[v].phase -= 1.0;
                }
            }

            // Normaliza e aplica volume
            mix *= (volume / 32767.0);

            // clamp
            if (mix > 1) mix = 1;
            if (mix < -1) mix = -1;

            buffer[i] = (short)(mix * 32767);
        }

        WAVEHDR header;
        header.dwBufferLength = BUFFER_SIZE * sizeof(short);
        header.lpData = (LPSTR)buffer;
        header.dwFlags = 0;

        waveOutPrepareHeader(hWave, &header, sizeof(header));
        waveOutWrite(hWave, &header, sizeof(header));

        Sleep(2);
        }

    waveOutClose(hWave);
    return 0;
}