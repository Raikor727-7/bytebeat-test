#include <stdio.h>
#include <windows.h>
#include <conio.h>  // pra _kbhit() e _getch()

int main() {
    printf("Piano simples — pressione teclas de A a K para tocar notas.\n");
    printf("Pressione ESC para sair.\n");

    while (1) {
        if (_kbhit()) {
            char tecla = _getch();

            if (tecla == 27) break; // ESC sai

            int freq = 0;
            switch (tecla) {
                case 'a': freq = 261; break; // Dó
                case 's': freq = 293; break; // Ré
                case 'd': freq = 329; break; // Mi
                case 'f': freq = 349; break; // Fá
                case 'g': freq = 392; break; // Sol
                case 'h': freq = 440; break; // Lá
                case 'j': freq = 493; break; // Si
                case 'k': freq = 523; break; // Dó (oitava acima)
                default: freq = 0; break;
            }

            if (freq > 0) {
                Beep(freq, 200); // 200 ms de duração
            }
        }
    }

    return 0;
}
