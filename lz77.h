typedef struct
{
    unsigned char offset; // 1 байт (0-255)
    unsigned char length; // 1 байт (0-255)
    char next_char;       // 1 байт
} LZ77Triple;

LZ77Triple *lz77_encode(const char *input, int window_left_size, int *num_triples, int window_right_size);
char *lz77_decode(const LZ77Triple *triples, int num_triples);
void write_triples(const char *filename, const LZ77Triple *triples, int num_triples);
LZ77Triple *read_triples(const char *filename, int *num_triples);
