#include <dos.h>
#include <conio.h>
#include <stdio.h>
#include <string.h>

#define DISK_SIZE 512

unsigned char virtual_disk[DISK_SIZE];
unsigned char buffer[DISK_SIZE];

void interrupt (*old_fdd_isr)(...);  

unsigned char crypto_key = 0xAA;
int disk_position = 0;

void xor_data(unsigned char *data, int size) {
    for (int i = 0; i < size; i++)
        data[i] ^= crypto_key;
}

void interrupt disk_interrupt(...) {
    if (disk_position > 0){
		int block_size = (disk_position < DISK_SIZE) ? disk_position : DISK_SIZE;
		xor_data(&virtual_disk[disk_position - block_size], block_size);
	}
	if (old_fdd_isr){
		old_fdd_isr();
	}
}

void enable_interrupt() {
    old_fdd_isr = getvect(0x26);
    setvect(0x26, disk_interrupt);
}

void disable_interrupt() {
    setvect(0x26, old_fdd_isr);
}

void write_to_disk(unsigned char *data, int size) {
    for (int i = 0; i < size && (disk_position + i) < DISK_SIZE; i++)
        virtual_disk[disk_position + i] = data[i];
    xor_data(&virtual_disk[disk_position], size);
    disk_position += size;
}

void read_from_disk(unsigned char *dest, int size) {
    for (int i = 0; i < size; i++)
        dest[i] = virtual_disk[i];
    xor_data(dest, size);
}

void view_disk() {
    int i, j;
    printf("\nСодержимое виртуального диска (ASCII):\n");
    printf("Offset:  00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n");
    printf("--------");
    for (i = 0; i < 16; i++) printf("---");
    printf("\n");

    for (i = 0; i < disk_position; i += 16) {
        printf("%04X:   ", i);
        for (j = 0; j < 16 && (i + j) < disk_position; j++)
            printf("%02X ", virtual_disk[i + j]);
        for (; j < 16; j++) printf("   ");
        printf(" ");
        for (j = 0; j < 16 && (i + j) < disk_position; j++) {
            unsigned char c = virtual_disk[i + j];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        printf("\n");

        if ((i / 16 + 1) % 16 == 0) {
            printf("Нажмите любую клавишу для продолжения...\n");
            getch();
        }
    }
}

int main() {
    char input[128];
    clrscr();
    enable_interrupt();

    while (1) {
        printf("\nМеню:\n");
        printf("1. Записать строку на диск\n");
        printf("2. Прочитать данные с диска\n");
        printf("3. Просмотреть содержимое виртуального диска\n");
        printf("4. Очистить диск\n");
        printf("5. Выход\n");
        printf("Выберите действие: ");
        char choice = getch();
        printf("%c\n", choice);

        switch(choice) {
            case '1': {
                printf("Введите строку: ");
                fgets(input, sizeof(input), stdin);
                size_t len = strlen(input);
                if (len > 0 && input[len-1] == '\n') input[len-1] = '\0';
                write_to_disk((unsigned char*)input, strlen(input));
                printf("Данные записаны.\n");
                break;
            }
            case '2': {
                read_from_disk(buffer, disk_position);
                printf("Прочитанные данные: ");
                for (int i = 0; i < disk_position; i++)
                    printf("%c", buffer[i]);
                printf("\n");
                break;
            }
            case '3':
                view_disk();
                break;
            case '4':
                disk_position = 0;
                printf("Диск очищен.\n");
                break;
            case '5':
                disable_interrupt();
                printf("Выход. ISR восстановлен.\n");
                return 0;
            default:
                printf("Неверный выбор!\n");
        }
    }
}