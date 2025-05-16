#include "lz77.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <ctype.h>
LZ77Triple *lz77_encode(const char *input, int window_left_size, int *num_triples, int window_right_size)
{
    int input_len = strlen(input);
    LZ77Triple *triples = malloc(sizeof(LZ77Triple) * (input_len + 1));
    int triple_count = 0;
    int pos = 0;
    // triples - указатель на массив структур
    while (pos < input_len)
    {
        unsigned char max_length = 0;
        unsigned char best_offset = 0;
        char next_char = input[pos];

        int window_start = pos - window_left_size;
        if (window_start < 0)
            window_start = 0;

        // Поиск наилучшего совпадения в окне
        for (int i = window_start; i < pos; i++)
        {
            int current_length = 0;
            // Проверяем совпадения с зацикливанием
            while (pos + current_length < input_len &&                                       // (1) Не вышли за границы строки
                   input[i + (current_length % (pos - i))] == input[pos + current_length] && // (2) Совпадение символов
                   current_length < window_right_size)                                       // (3) Ограничение длины совпадения
            {
                current_length++;
            }
            if (current_length > max_length && (pos - i) <= window_left_size)
            {
                max_length = current_length;
                best_offset = pos - i;
            }
        }

        // Формирование тройки
        if (max_length > 0)
        {
            if (pos + max_length < input_len)
            {
                next_char = input[pos + max_length];
            }
            else
            {
                // когда весь массив кончится
                next_char = '\0';
                max_length = input_len - pos;
            }
            triples[triple_count] = (LZ77Triple){best_offset, max_length, next_char};
            pos += max_length + 1;
        }
        else
        {
            triples[triple_count] = (LZ77Triple){0, 0, input[pos]};
            pos++;
        }

        // printf("Triple %d: (offset=%d, length=%d, next_char=", triple_count + 1,
        //        triples[triple_count].offset, triples[triple_count].length);
        // if (triples[triple_count].next_char == '\0')
        // {
        //     printf("_");
        // }
        // else if (isprint(triples[triple_count].next_char))
        // {
        //     printf("%c", triples[triple_count].next_char);
        // }
        // printf(")\n");

        triple_count++;
    }

    *num_triples = triple_count;
    return triples;
}

char *lz77_decode(const LZ77Triple *triples, int num_triples)
{
    char *output = malloc(1);
    output[0] = '\0';
    int output_len = 0;
    for (int i = 0; i < num_triples; i++)
    {
        LZ77Triple t = triples[i];
        if (t.offset == 0 && t.length == 0) // новый символ
        {
            output = realloc(output, output_len + 2); // увеличение буфера на 2 байта
            output[output_len++] = t.next_char;
            output[output_len] = '\0';
        }
        else
        {
            int start = output_len - t.offset; // откуда копировать
            for (int j = 0; j < t.length; j++) // копирует length символов из уже декодированной части
            {
                if (start + j < 0 || start + j >= output_len)
                {
                    fprintf(stderr, "Decode error: invalid offset\n");
                    free(output);
                    return NULL;
                }
                output = realloc(output, output_len + 2);
                output[output_len++] = output[start + j];
                output[output_len] = '\0';
            }
            if (t.next_char != '\0')
            {
                output = realloc(output, output_len + 2);
                output[output_len++] = t.next_char;
                output[output_len] = '\0';
            }
        }
        // printf("\n");
        // printf("num_triples: %d\ni: %d\nt.offset: %d\nt.length: %d\nt.next_char: %d\noutput[output_len]: %d\nsizeof(output): %d\noutput_len: %d\nt: %d\n", num_triples, i, t.offset, t.length, t.next_char, output[output_len - 1], sizeof(char) * output_len, output_len, t);
    }
    return output;
}

void write_triples(const char *filename, const LZ77Triple *triples, int num_triples)
{
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        perror("Failed to open file");
        return;
    }
    // fwrite(указ на первый объект из массива который записывается, размер каждого объекта, количество объектов, которые надо записать, указатель на файловый поток для записи)
    fwrite(triples, sizeof(LZ77Triple), num_triples, file);
    fclose(file);
}

LZ77Triple *read_triples(const char *filename, int *num_triples)
{
    FILE *file = fopen(filename, "rb");
    if (!file)
    {
        perror("Failed to open file");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file); // размер файла в байт
    fseek(file, 0, SEEK_SET);

    *num_triples = file_size / sizeof(LZ77Triple); // кол-во троек
    LZ77Triple *triples = malloc(file_size);

    if (fread(triples, sizeof(LZ77Triple), *num_triples, file) != *num_triples)
    {
        perror("Error reading triples");
        free(triples);
        fclose(file);
        return NULL;
    }

    fclose(file);
    return triples;
}