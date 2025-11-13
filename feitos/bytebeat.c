// bytebeat_with_visual.c
// Bytebeat + visual simples em console (Windows)
// Compilar: gcc bytebeat_with_visual.c -o bytebeat_with_visual -lwinmm

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define RATE 8000
#define COLS 80     // largura do "visual" no console
#define ROWS 24     // altura do "visual" no console

// Função que posiciona o cursor no console (x=col, y=row)
static void set_cursor_pos(short x, short y) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD pos = { x, y };
    SetConsoleCursorPosition(h, pos);
}

// Função que limpa a tela e esconde o cursor
static void console_setup() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(h, &csbi);

    // Limpa a tela inteira
    DWORD written;
    COORD origin = {0,0};
    DWORD size = csbi.dwSize.X * csbi.dwSize.Y;
    FillConsoleOutputCharacterA(h, ' ', size, origin, &written);
    FillConsoleOutputAttribute(h, csbi.wAttributes, size, origin, &written);
    set_cursor_pos(0,0);

    // Esconder cursor
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(h, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(h, &cursorInfo);
}

// Restaura cursor visibilidade (chamar antes de sair, se quiser)
static void console_restore_cursor() {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(h, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(h, &cursorInfo);
}

int main() {
    // Inicializa áudio (WAVE)
    HWAVEOUT hWaveOut;
    WAVEFORMATEX wfx = {0};
    wfx.nSamplesPerSec = RATE;
    wfx.wBitsPerSample = 8;
    wfx.nChannels = 1;
    wfx.cbSize = 0;
    wfx.wFormatTag = WAVE_FORMAT_PCM;
    wfx.nBlockAlign = wfx.nChannels * wfx.wBitsPerSample / 8;
    wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

    if (waveOutOpen(&hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
        printf("Erro ao abrir dispositivo de áudio.\n");
        return 1;
    }

    // Buffer por segundo (8000 samples de 8-bit)
    unsigned char buffer[RATE];

    // Preparação do console
    console_setup();
    printf("Bytebeat + Visual (CTRL+C para sair)\n");
    // Reserva 1 linha para título, o visual ocupa ROWS linhas abaixo
    for (int r = 0; r < ROWS; ++r) {
        printf("\n");
    }
    // Mantém o resto do texto sob o visual
    printf("\n"); // linha extra para instruções

    // Loop principal: gera sample a sample, envia buffer a cada RATE samples
    for (unsigned long t = 0;; t++) {
        // --- expressão Bytebeat (mude aqui pra experimentar) ---
        // Exemplo clássico:
        unsigned char sample = (unsigned char)( (t * ((t >> 5) | (t >> 8))) >> (t >> 16) );
        // -----------------------------------------------------
        buffer[t % RATE] = sample;

        // Quando enche 1 segundo de buffer, envia para o áudio e atualiza visual
        if (t % RATE == RATE - 1) {
            // Envia áudio
            WAVEHDR header = {0};
            header.lpData = (LPSTR)buffer;
            header.dwBufferLength = RATE;
            if (waveOutPrepareHeader(hWaveOut, &header, sizeof(header)) == MMSYSERR_NOERROR) {
                waveOutWrite(hWaveOut, &header, sizeof(header));
            }

            // --- Visual simples ---
            // Mapear o buffer em COLS colunas, cada coluna é a média de alguns samples
            int samples_per_col = RATE / COLS;
            if (samples_per_col < 1) samples_per_col = 1;

            // Calcula médias por coluna
            int cols_avg[COLS];
            for (int c = 0; c < COLS; ++c) {
                unsigned int sum = 0;
                int start = c * samples_per_col;
                int end = start + samples_per_col;
                if (end > RATE) end = RATE;
                for (int i = start; i < end; ++i) sum += buffer[i];
                cols_avg[c] = (int)(sum / (end - start + 0.0));
            }

            // Desenha no console: usa ROWS linhas (0 = topo)
            // Vamos posicionar o cursor na linha 1 (logo abaixo do título)
            set_cursor_pos(0, 1);
            // Para cada linha (topo→base)
            for (int r = 0; r < ROWS; ++r) {
                // construir a string da linha
                char line[COLS + 1];
                for (int c = 0; c < COLS; ++c) {
                    // mapa: 0..255 -> 0..ROWS-1 (0 = base). Queremos 'altura' proporcional.
                    int height = (cols_avg[c] * ROWS) / 256; // 0..ROWS
                    // Para desenhar vertical bar bottom-aligned: se (row index from top) >= ROWS - height -> desenha
                    int row_from_top = r;
                    if (row_from_top >= ROWS - height) line[c] = '#';
                    else line[c] = ' ';
                }
                line[COLS] = '\0';
                // imprime a linha (mantendo cursor na coluna 0)
                printf("%s\n", line);
            }

            // imprime uma pequena legenda / nível médio
            unsigned long avg_all = 0;
            for (int i = 0; i < RATE; ++i) avg_all += buffer[i];
            avg_all /= RATE;
            printf("Média do último buffer: %lu (0-255). Fórmula atual em código.\n", avg_all);

            // Pausa curta correspondente ao segundo (pode remover se preferir)
            Sleep(1000 * RATE / wfx.nSamplesPerSec); // equivale a Sleep(1000)
        }
    }

    // nunca alcançado neste loop infinito, mas bom ter por completude
    waveOutClose(hWaveOut);
    console_restore_cursor();
    return 0;
}
