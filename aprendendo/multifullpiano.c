#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <mmsystem.h>
#include <math.h>
#include <conio.h>  

int main(){
    #define BUFFER_SIZE (44100 / 20)
    #define MAX_VOICES 5
    #define PI 3.14159265358979323

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

    int play_type = 0;
    int volume = 28000;
    float duty_cycle = 0.3;

    while (1) {
        if (_kbhit()){
            char tecla = _getch();
            switch (tecla)
            {
            case '1':
                system("cls");
                printf("modo sine\n");
                play_type = 0;
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

        // Controles de volume e duty cycle
        if (GetAsyncKeyState(VK_UP)){
            volume += 500;
            if (volume >= 32767){
                volume = 32767;
                printf("\rVolume: %d    volume maximo", volume);
            } else {
                printf("\rVolume: %d                        ", volume); 
            }
            Sleep(2);      
        }
        if (GetAsyncKeyState(VK_DOWN)){
            volume -= 500;
            if (volume <= 5000){
                volume = 5000;
                printf("\rVolume: %d    volume minimo", volume);
            } else {
                printf("\rVolume: %d                        ", volume); 
            }
            Sleep(2);
        }
        if (GetAsyncKeyState(VK_RIGHT)){
            duty_cycle += 0.1;
            if (duty_cycle >= 1){
                duty_cycle = 1;
                printf("\rduty cycle: %f    max", duty_cycle);
            } else {
                printf("\rduty cycle: %f          ", duty_cycle); 
            }
            Sleep(200);
        }
        if (GetAsyncKeyState(VK_LEFT)){
            duty_cycle -= 0.1;
            if (duty_cycle <= 0){
                duty_cycle = 0;
                printf("\rduty cycle: %f    min", duty_cycle);
            } else {
                printf("\rduty cycle: %f          ", duty_cycle); 
            }
            Sleep(200);                
        }

        if (GetAsyncKeyState(VK_ESCAPE)) break;

        // Ativação de notas
        if (GetAsyncKeyState('A')){ 
            for(int v = 0; v < MAX_VOICES; v++){
                if(!voices[v].active){
                    voices[v].freq = 261.63; //DO
                    voices[v].active = 1;
                    voices[v].phase = 0;
                    break;
                }
            }
        }
        if (GetAsyncKeyState('S')){ 
            for(int v = 0; v < MAX_VOICES; v++){
                if(!voices[v].active){
                    voices[v].freq = 293.66; //RE
                    voices[v].active = 1;
                    voices[v].phase = 0;
                    break;
                }
            }
        }
        if (GetAsyncKeyState('D')){ 
            for(int v = 0; v < MAX_VOICES; v++){
                if(!voices[v].active){
                    voices[v].freq = 329.63; //MI
                    voices[v].active = 1;
                    voices[v].phase = 0;
                    break;
                }
            }
        }
        if (GetAsyncKeyState('F')){ 
            for(int v = 0; v < MAX_VOICES; v++){
                if(!voices[v].active){
                    voices[v].freq = 349.23; //FA
                    voices[v].active = 1;
                    voices[v].phase = 0;
                    break;
                }
            }
        }
        if (GetAsyncKeyState('G')){ 
            for(int v = 0; v < MAX_VOICES; v++){
                if(!voices[v].active){
                    voices[v].freq = 392.00; //SOL
                    voices[v].active = 1;
                    voices[v].phase = 0;
                    break;
                }
            }
        }
        if (GetAsyncKeyState('H')){ 
            for(int v = 0; v < MAX_VOICES; v++){
                if(!voices[v].active){
                    voices[v].freq = 440.00; //LA
                    voices[v].active = 1;
                    voices[v].phase = 0;
                    break;
                }
            }
        }
        if (GetAsyncKeyState('J')){ 
            for(int v = 0; v < MAX_VOICES; v++){
                if(!voices[v].active){
                    voices[v].freq = 493.88; //SI
                    voices[v].active = 1;
                    voices[v].phase = 0;
                    break;
                }
            }
        }
        if (GetAsyncKeyState('K')){ 
            for(int v = 0; v < MAX_VOICES; v++){
                if(!voices[v].active){
                    voices[v].freq = 261.63 * 2;//DO UMA OITAVA ACIMA
                    voices[v].active = 1;
                    voices[v].phase = 0;
                    break;
                }
            }
        }
        if (GetAsyncKeyState('Q')){ 
            for(int v = 0; v < MAX_VOICES; v++){
                if(!voices[v].active){
                    voices[v].freq = 261.63/2;//DO UMA OITAVA ABAIXO
                    voices[v].active = 1;
                    voices[v].phase = 0;
                    break;
                }
            }
        } 
        // ... (outras notas - manter como estão)

        // Desativação de notas
        if (!GetAsyncKeyState('A')) { for(int v = 0; v < MAX_VOICES; v++) if(voices[v].freq == 261.63) voices[v].active = 0; }
        if (!GetAsyncKeyState('S')) { for(int v = 0; v < MAX_VOICES; v++) if(voices[v].freq == 293.66) voices[v].active = 0; }
        if (!GetAsyncKeyState('D')) { for(int v = 0; v < MAX_VOICES; v++) if(voices[v].freq == 329.63) voices[v].active = 0; }
        if (!GetAsyncKeyState('F')) { for(int v = 0; v < MAX_VOICES; v++) if(voices[v].freq == 349.23) voices[v].active = 0; }
        if (!GetAsyncKeyState('G')) { for(int v = 0; v < MAX_VOICES; v++) if(voices[v].freq == 392.00) voices[v].active = 0; }
        if (!GetAsyncKeyState('H')) { for(int v = 0; v < MAX_VOICES; v++) if(voices[v].freq == 440.00) voices[v].active = 0; }
        if (!GetAsyncKeyState('J')) { for(int v = 0; v < MAX_VOICES; v++) if(voices[v].freq == 493.88) voices[v].active = 0; }
        if (!GetAsyncKeyState('K')) { for(int v = 0; v < MAX_VOICES; v++) if(voices[v].freq == 261.63*2) voices[v].active = 0; }
        if (!GetAsyncKeyState('Q')) { for(int v = 0; v < MAX_VOICES; v++) if(voices[v].freq == 261.63/2) voices[v].active = 0; }
        // ... (outras desativações - manter como estão)

        // VERIFICAR SE HÁ VOZES ATIVAS (CORREÇÃO PRINCIPAL)
        int any_voice_active = 0;
        for(int v = 0; v < MAX_VOICES; v++) {
            if(voices[v].active) {
                any_voice_active = 1;
                break;
            }
        }

        if (any_voice_active) {
            short buffer[BUFFER_SIZE];
            
            // Inicializar buffer com silêncio
            for (int i = 0; i < BUFFER_SIZE; i++) {
                buffer[i] = 0;
            }

            if(play_type == 0){ // MODO SINE
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    double mixed_sample = 0.0;
                    int active_count = 0;
                    
                    for (int v = 0; v < MAX_VOICES; v++) {
                        if (voices[v].active) {
                            // Gera onda para esta voz
                            double sample = sin(voices[v].phase);
                            mixed_sample += sample;
                            active_count++;
                            
                            // Atualiza fase
                            voices[v].phase += 2.0 * PI * voices[v].freq / 44100.0;
                            if (voices[v].phase >= 2.0 * PI) voices[v].phase -= 2.0 * PI;
                        }
                    }
                    
                    // Aplica volume (com normalização se houver múltiplas vozes)
                    if (active_count > 0) {
                        buffer[i] = (short)(volume * mixed_sample / active_count);
                    }
                }
            } 
            else if (play_type == 1){ // MODO QUADRADO
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    double mixed_sample = 0.0;
                    int active_count = 0;
                    
                    for (int v = 0; v < MAX_VOICES; v++) {
                        if (voices[v].active) {
                            double t = fmod(voices[v].phase / (2.0 * PI), 1.0);
                            double sample = (t < 0.5) ? 1.0 : -1.0;
                            mixed_sample += sample;
                            active_count++;
                            
                            voices[v].phase += 2.0 * PI * voices[v].freq / 44100.0;
                        }
                    }
                    
                    if (active_count > 0) {
                        buffer[i] = (short)(volume * mixed_sample / active_count);
                    }
                }
            }
            else if (play_type == 2){ // MODO SERRA
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    double mixed_sample = 0.0;
                    int active_count = 0;
                    
                    for (int v = 0; v < MAX_VOICES; v++) {
                        if (voices[v].active) {
                            double t = fmod(voices[v].phase / (2.0 * PI), 1.0);
                            double sample = 2.0 * t - 1.0;
                            mixed_sample += sample;
                            active_count++;
                            
                            voices[v].phase += 2.0 * PI * voices[v].freq / 44100.0;
                        }
                    }
                    
                    if (active_count > 0) {
                        buffer[i] = (short)(volume * mixed_sample / active_count);
                    }
                }
            }
            else if (play_type == 3){ // MODO TRIANGULO
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    double mixed_sample = 0.0;
                    int active_count = 0;
                    
                    for (int v = 0; v < MAX_VOICES; v++) {
                        if (voices[v].active) {
                            double t = fmod(voices[v].phase / (2.0 * PI), 1.0);
                            double sample = 2.0 * fabs(2.0 * t - 1.0) - 1.0;
                            mixed_sample += sample;
                            active_count++;
                            
                            voices[v].phase += 2.0 * PI * voices[v].freq / 44100.0;
                        }
                    }
                    
                    if (active_count > 0) {
                        buffer[i] = (short)(volume * mixed_sample / active_count);
                    }
                }
            }
            else if (play_type == 4){ // MODO QUADRADO COM DUTY CYCLE
                for (int i = 0; i < BUFFER_SIZE; i++) {
                    double mixed_sample = 0.0;
                    int active_count = 0;
                    
                    for (int v = 0; v < MAX_VOICES; v++) {
                        if (voices[v].active) {
                            double t = fmod(voices[v].phase / (2.0 * PI), 1.0);
                            double sample = (t < duty_cycle) ? 1.0 : -1.0;
                            mixed_sample += sample;
                            active_count++;
                            
                            voices[v].phase += 2.0 * PI * voices[v].freq / 44100.0;
                        }
                    }
                    
                    if (active_count > 0) {
                        buffer[i] = (short)(volume * mixed_sample / active_count);
                    }
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
            
            // Pequena pausa para evitar sobrecarga
            Sleep(5);
        } else {
            // Pequena pausa quando não há áudio para tocar
            Sleep(10);
        }
    }

    waveOutClose(hWave);
    return 0;
}