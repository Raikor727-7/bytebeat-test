// compile: gcc multiaudio.c -o multiaudio -lwinmm
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <windows.h>
#include <mmsystem.h>
#include <conio.h>
#include <time.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 44100
#define BITS_PER_SAMPLE 16
#define CHANNELS 1
#define BUFFER_SECONDS (1.0/20.0) // 0.05s buffer
#define BUFFER_SAMPLES ((int)(SAMPLE_RATE * BUFFER_SECONDS))

// Shared state
volatile int piano_on = 0;
volatile double piano_freq = 440.0;
volatile int trigger_kick = 0;
volatile int trigger_snare = 0;
volatile int trigger_hat = 0;

// protect shared writes
CRITICAL_SECTION state_cs;

// Per-voice runtime state (kept between buffers)
typedef struct {
    double phase;
    double env;       // envelope amplitude (0..1)
    double env_decay; // decay factor per sample
    int active;
} VoiceState;

VoiceState piano_state = {0};
VoiceState kick_state = {0};
VoiceState snare_state = {0};
VoiceState hat_state = {0};

HWAVEOUT hWave = NULL;
volatile int audio_running = 1;

// utility clamp
static inline short clamp16(int v){
    if (v > 32767) return 32767;
    if (v < -32768) return -32768;
    return (short)v;
}

// simple noise [-1..1]
static inline double noise01(){
    return ((rand() / (double)RAND_MAX) * 2.0 - 1.0);
}

// start instrument envelopes/params (called under CS)
void start_kick(){
    kick_state.active = 1;
    kick_state.phase = 0.0;
    kick_state.env = 1.0;
    // decay chosen empirically
    kick_state.env_decay = 0.0009; // bigger = faster decay
}
void start_snare(){
    snare_state.active = 1;
    snare_state.phase = 0.0;
    snare_state.env = 1.0;
    snare_state.env_decay = 0.004;
}
void start_hat(){
    hat_state.active = 1;
    hat_state.phase = 0.0;
    hat_state.env = 1.0;
    hat_state.env_decay = 0.02;
}

// audio thread: generate, mix and play buffers
DWORD WINAPI audio_thread_func(LPVOID arg){
    (void)arg;
    WAVEFORMATEX wfx = {0};
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nChannels = CHANNELS;
    wfx.nSamplesPerSec = SAMPLE_RATE;
    wfx.wBitsPerSample = BITS_PER_SAMPLE;
    wfx.nBlockAlign = (wfx.nChannels * wfx.wBitsPerSample) / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
    wfx.cbSize = 0;

    // open device (open here to ensure thread context)
    if (waveOutOpen(&hWave, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR){
        printf("Failed to open waveOut\n");
        return 1;
    }

    while (audio_running){
        // allocate buffer for this block
        int samples = BUFFER_SAMPLES;
        short *buf = (short*)malloc(samples * sizeof(short));
        if (!buf) break;

        // copy shared state under lock
        EnterCriticalSection(&state_cs);
        int local_piano_on = piano_on;
        double local_piano_freq = piano_freq;
        int local_trigger_kick = trigger_kick;
        int local_trigger_snare = trigger_snare;
        int local_trigger_hat = trigger_hat;
        // clear triggers (one-shot)
        trigger_kick = trigger_snare = trigger_hat = 0;
        LeaveCriticalSection(&state_cs);

        // if a trigger, start the corresponding envelope
        if (local_trigger_kick) start_kick();
        if (local_trigger_snare) start_snare();
        if (local_trigger_hat) start_hat();

        // generate per-sample
        for (int i=0;i<samples;i++){
            double sample_acc = 0.0;

            // Piano (sine) - sustained while piano_on
            if (local_piano_on || piano_state.active){
                if (local_piano_on && !piano_state.active){
                    // start envelope
                    piano_state.active = 1;
                    piano_state.env = 1.0;
                    piano_state.env_decay = 0.0006; // slowish decay for sustain release
                }
                if (piano_state.active){
                    // sin oscillator
                    double phase_inc = 2.0 * M_PI * local_piano_freq / SAMPLE_RATE;
                    double s = sin(piano_state.phase);
                    sample_acc += s * piano_state.env * 8000.0; // piano volume
                    piano_state.phase += phase_inc;
                    if (piano_state.phase > 2.0*M_PI) piano_state.phase -= 2.0*M_PI;
                    // if piano is released (main thread toggled off) start stronger decay
                    EnterCriticalSection(&state_cs);
                    int still_on = piano_on;
                    LeaveCriticalSection(&state_cs);
                    if (!still_on){
                        // faster decay when released
                        piano_state.env *= 0.996;
                        if (piano_state.env < 0.0005) piano_state.active = 0;
                    } else {
                        // slight sustain: no big change
                        // optional: implement ADSR here
                    }
                }
            }

            // Kick (sine with falling pitch + envelope)
            if (kick_state.active){
                // falling pitch effect
                double pitch = 80.0 * (1.0 - 0.5 * (1.0 - kick_state.env)); // simple mapping
                double phase_inc = 2.0 * M_PI * pitch / SAMPLE_RATE;
                double s = sin(kick_state.phase);
                sample_acc += s * kick_state.env * 12000.0;
                kick_state.phase += phase_inc;
                if (kick_state.phase > 2.0*M_PI) kick_state.phase -= 2.0*M_PI;
                // decay envelope
                kick_state.env *= (1.0 - kick_state.env_decay);
                if (kick_state.env < 0.0001) kick_state.active = 0;
            }

            // Snare (noise + short body)
            if (snare_state.active){
                double s_noise = noise01();
                double body = sin(snare_state.phase);
                sample_acc += s_noise * snare_state.env * 8000.0;
                sample_acc += body * snare_state.env * 3000.0;
                snare_state.phase += 2.0*M_PI*180.0/SAMPLE_RATE;
                if (snare_state.phase > 2.0*M_PI) snare_state.phase -= 2.0*M_PI;
                snare_state.env *= (1.0 - snare_state.env_decay);
                if (snare_state.env < 0.0001) snare_state.active = 0;
            }

            // Hat (fast noise)
            if (hat_state.active){
                double s_noise = noise01();
                sample_acc += s_noise * hat_state.env * 4000.0;
                hat_state.env *= (1.0 - hat_state.env_decay);
                if (hat_state.env < 0.0001) hat_state.active = 0;
            }

            // clamp and store
            buf[i] = clamp16((int)sample_acc);
        }

        // prepare header and write (single-threaded here)
        WAVEHDR header = {0};
        header.lpData = (LPSTR)buf;
        header.dwBufferLength = samples * sizeof(short);
        waveOutPrepareHeader(hWave, &header, sizeof(WAVEHDR));
        waveOutWrite(hWave, &header, sizeof(WAVEHDR));

        // wait until finished, then cleanup
        while (waveOutUnprepareHeader(hWave, &header, sizeof(WAVEHDR)) == WAVERR_STILLPLAYING){
            Sleep(1);
        }
        free(buf);
    }

    waveOutReset(hWave);
    waveOutClose(hWave);
    return 0;
}

int main(){
    InitializeCriticalSection(&state_cs);
    srand((unsigned int)time(NULL));

    // start audio thread
    DWORD tid;
    HANDLE hThread = CreateThread(NULL, 0, audio_thread_func, NULL, 0, &tid);
    if (!hThread){
        printf("Failed to create audio thread\n");
        return 1;
    }

    printf("Piano+Bateria: A=toggle piano (freq A=440Hz, S=523Hz), G=kick, F=snare, H=hat, ESC=quit\n");

    // small UI loop: toggle piano and triggers
    int piano_toggle = 0;
    while (1){
        if (_kbhit()){
            char c = _getch();
            if (c == 27) break;
            EnterCriticalSection(&state_cs);
            if (c == 'a' || c == 'A') { piano_toggle = !piano_toggle; piano_on = piano_toggle; piano_freq = 440.0; }
            else if (c == 's' || c == 'S') { piano_toggle = !piano_toggle; piano_on = piano_toggle; piano_freq = 523.25; }
            else if (c == 'g' || c == 'G') { trigger_kick = 1; }
            else if (c == 'f' || c == 'F') { trigger_snare = 1; }
            else if (c == 'h' || c == 'H') { trigger_hat = 1; }
            LeaveCriticalSection(&state_cs);
        }
        Sleep(5);
    }

    // stop audio
    audio_running = 0;
    WaitForSingleObject(hThread, 2000);
    DeleteCriticalSection(&state_cs);
    CloseHandle(hThread);
    return 0;
}
