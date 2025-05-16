#include "lz77.h"
#include <stdio.h>
#include <stdlib.h>

void print_menu()
{
    printf("\nMenu:\n");
    printf("1. Compress file\n");
    printf("2. Decompress file\n");
    printf("3. Exit\n");
    printf("Choose: ");
}

void compress_file()
{
    char input_path[256], output_path[256];
    printf("Input file: ");
    scanf("%255s", input_path);
    printf("Output file: ");
    scanf("%255s", output_path);
    printf("\n");
    FILE *file = fopen(input_path, "rb");
    if (!file)
    {
        perror("Error opening file");
        return;
    }
    fseek(file, 0, SEEK_END);
    long original_size = ftell(file); // размер original файла. ftell берет длину слева от указателя в данный момент в массиве и подсчитывает размер в байтах
    fseek(file, 0, SEEK_SET);
    char *content = malloc(original_size + 1); // + символ конца
    fread(content, 1, original_size, file);    // (массив куда записывать, размер одного символа в байтах, общий размер, массив откуда записывать)
    content[original_size] = '\0';             // символ конца строки (терминальный ноль)
    fclose(file);

    
    // ====================================================================================================================================================
    //  аддаптивность кода (перебор 10 различных значений скользящей строки)
    struct CompressionResult
    {
        LZ77Triple *triples;
        int num_triples;
        int window_size;
        long compressed_size;
    } results[10];

    //  10 разных размеров окон от 4 до 255 с равным шагом
    for (int i = 0; i < 10; i++)
    {
        int window_left_size = 0, window_right_size = 0;
        int window_size;
        if (i == 0)
        {
            window_left_size = 6, window_right_size = 4;
            window_size = window_left_size + window_right_size;
        }
        else
        {
            window_left_size = 4 + (i * 27);
            window_right_size = 4 + (i * 27);
            if (window_left_size > 255)
                window_left_size = 255;
            if (window_right_size > 255)
                window_right_size = 255;
        }
        int num_triples;
        LZ77Triple *triples = lz77_encode(content, window_left_size, &num_triples, window_right_size);
        // сжатие файла
        // на вход: массив символов, размер скользящего окна слева, адрес куда записывать номер тройки, размер скольз окна справа)
        // возвращает массив структур троек (словарь, буфер, next char)

        window_size = window_left_size + window_right_size;
        // Сохраняем результат

        results[i].triples = triples;
        results[i].num_triples = num_triples;
        results[i].window_size = window_size;

        // Временный файл для оценки размера
        char temp_path[256];
        sprintf(temp_path, "%s.temp%d", output_path, i);
        write_triples(temp_path, triples, num_triples); // запись троек во временный файл

        FILE *temp_file = fopen(temp_path, "rb");
        fseek(temp_file, 0, SEEK_END);
        results[i].compressed_size = ftell(temp_file);
        fclose(temp_file);
        remove(temp_path); // Удаляем временный файл

        printf("Test %d: window_size=%d, compressed_size=%ld\n",
                i + 1, window_size, results[i].compressed_size);
    }

    // Находим результат с наименьшим размером
    int best_index = 0;
    for (int i = 1; i < 10; i++)
    {
        if (results[i].compressed_size < results[best_index].compressed_size)
        {
            best_index = i;
        }
    }

    printf("\nBest compression: window_size=%d, compressed_size=%ld\n",
           results[best_index].window_size, results[best_index].compressed_size);

    // Записываем лучший результат
    write_triples(output_path, results[best_index].triples, results[best_index].num_triples);

    // Освобождаем память
    free(content);
    for (int i = 0; i < 10; i++)
    {
        if (i != best_index)
        { // Лучший результат ещё нужен для вывода
            free(results[i].triples);
        }
    }

    // Получение размера сжатого файла
    FILE *compressed = fopen(output_path, "rb");
    fseek(compressed, 0, SEEK_END);
    long compressed_size = ftell(compressed);
    fclose(compressed);

    // размеры файлов и насколько сжато
    printf("Original size: %d bytes\n", original_size);
    printf("Compressed size: %d bytes\n", compressed_size);
    printf("Compression ratio: %.2f\n", (double)compressed_size / original_size);

    free(results[best_index].triples);
}
void decompress_file()
{
    char input_path[256], output_path[256];
    printf("Input file: ");
    scanf("%255s", input_path);
    printf("Output file: ");
    scanf("%255s", output_path);

    int num_triples;
    LZ77Triple *triples = read_triples(input_path, &num_triples);
    if (!triples)
        return;

    // принимает на вход дин. массив структур, число трроек
    // возвращает декодированный массив
    char *decoded = lz77_decode(triples, num_triples);
    if (!decoded)
    {
        free(triples);
        return;
    }

    FILE *file = fopen(output_path, "wb");
    fputs(decoded, file); // массив в файл
    fclose(file);

    free(triples);
    free(decoded);
}

int main()
{
    int choice;
    do
    {
        print_menu();
        scanf("%d", &choice);
        while (getchar() != '\n')
            ;

        switch (choice)
        {
        case 1:
            compress_file();
            break;
        case 2:
            decompress_file();
            break;
        case 3:
            printf("Exiting...\n");
            break;
        default:
            printf("Invalid choice\n");
        }
    } while (choice != 3);

    return 0;
}