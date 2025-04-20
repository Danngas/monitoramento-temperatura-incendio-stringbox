#include <stdio.h>
#include <stdint.h>

#define MATRIX_ROWS 5
#define MATRIX_COLS 5
#define MATRIX_DEPTH 3
#define NUMERO_DE_IMAGENS 10 // ajuste para a quantidade de imagens do seu

// Função para converter valores ARGB (0xAARRGGBB) para RGB
void convertToRGB(int argb, int rgb[3])
{
    rgb[0] = argb & 0xFF;         // Blue
    rgb[2] = (argb >> 16) & 0xFF; // Red
    rgb[1] = (argb >> 8) & 0xFF;  // Green
}

// Função para processar uma matriz ARGB para RGB
void processMatrix(const uint32_t input[MATRIX_ROWS * MATRIX_COLS], int output[MATRIX_ROWS][MATRIX_COLS][MATRIX_DEPTH])
{
    for (int i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++)
    {
        int rgb[3];
        convertToRGB(input[i], rgb);

        int row = i / MATRIX_COLS; // Cálculo da linha
        int col = i % MATRIX_COLS; // Cálculo da coluna

        // Armazenar os valores RGB na matriz 5x5x3
        output[row][col][0] = rgb[0]; // Red
        output[row][col][1] = rgb[1]; // Green
        output[row][col][2] = rgb[2]; // Blue
    }
}

// Função para exibir uma matriz RGB
void printMatrix(int matrix[MATRIX_ROWS][MATRIX_COLS][MATRIX_DEPTH])
{
    printf("{\n");
    for (int i = 0; i < MATRIX_ROWS; i++)
    {
        printf("    {");
        for (int j = 0; j < MATRIX_COLS; j++)
        {
            printf("{%d, %d, %d}", matrix[i][j][0], matrix[i][j][1], matrix[i][j][2]);
            if (j < MATRIX_COLS - 1)
            {
                printf(", ");
            }
        }
        printf("}");
        if (i < MATRIX_ROWS - 1)
        {
            printf(",");
        }
        printf("\n");
    }
    printf("}\n");
}

int main()
{
    static const uint32_t new_piskel_data[NUMERO_DE_IMAGENS][25] = {
        {0x00000001, 0xff0101ff, 0xff0001ff, 0xff0100fe, 0x00010100,
         0x00000001, 0xff0001ff, 0x00010100, 0xff0000fe, 0x00010001,
         0x00000000, 0xff0101fe, 0x00000000, 0xff0000fe, 0x00010000,
         0x00000001, 0xff0101ff, 0x00000001, 0xff0100fe, 0x00010001,
         0x00010100, 0xff0001fe, 0xff0000ff, 0xff0000ff, 0x00000000},
        {0x00010101, 0x00000000, 0x00010100, 0xff0000fe, 0x00010000,
         0x00000000, 0x00010101, 0x00000001, 0xff0100fe, 0x00000001,
         0x00010001, 0x00010000, 0x00000101, 0xff0100ff, 0x00000101,
         0x00000001, 0x00010001, 0x00000000, 0xff0101ff, 0x00010000,
         0x00010001, 0x00000001, 0x00000000, 0xff0001fe, 0x00000001},
        {0x00000100, 0xff0101fe, 0xff0100ff, 0xff0000ff, 0x00000100,
         0x00000100, 0x00010001, 0x00010101, 0xff0001fe, 0x00010001,
         0x00010001, 0xff0000ff, 0xff0101ff, 0xff0000ff, 0x00010100,
         0x00000001, 0xff0100ff, 0x00000000, 0x00010000, 0x00010000,
         0x00010100, 0xff0001ff, 0xff0100fe, 0xff0100fe, 0x00000100},
        {0x00010100, 0xff0000ff, 0xff0001ff, 0xff0001ff, 0x00010001,
         0x00000100, 0x00000101, 0x00000000, 0xff0001fe, 0x00010100,
         0x00000100, 0xff0000fe, 0xff0101ff, 0xff0100fe, 0x00010000,
         0x00000100, 0x00010000, 0x00010000, 0xff0101fe, 0x00010101,
         0x00010000, 0xff0101ff, 0xff0101ff, 0xff0100ff, 0x00000100},
        {0x00010000, 0xff0101fe, 0x00010100, 0xff0100ff, 0x00010001,
         0x00010000, 0xff0001ff, 0x00000000, 0xff0001fe, 0x00010100,
         0x00010000, 0xff0100ff, 0xff0001ff, 0xff0000ff, 0x00000001,
         0x00010000, 0x00010101, 0x00000000, 0xff0101fe, 0x00000001,
         0x00000101, 0x00010000, 0x00000001, 0xff0000fe, 0x00000101},
        {0x00010101, 0xff0100fe, 0xff0100fe, 0xff0101fe, 0x00000001,
         0x00010101, 0xff0101ff, 0x00010100, 0x00000101, 0x00000000,
         0x00010000, 0xff0100ff, 0xff0000ff, 0xff0100ff, 0x00000100,
         0x00000000, 0x00000000, 0x00010000, 0xff0101fe, 0x00000101,
         0x00000101, 0xff0000fe, 0xff0100fe, 0xff0100ff, 0x00000000},
        {0x00010101, 0xff0001ff, 0xff0101ff, 0xff0101ff, 0x00000100,
         0x00000001, 0xff0001fe, 0x00000000, 0x00010101, 0x00010100,
         0x00010101, 0xff0101ff, 0xff0001ff, 0xff0000fe, 0x00000000,
         0x00010101, 0xff0001ff, 0x00000100, 0xff0001ff, 0x00000100,
         0x00000000, 0xff0101fe, 0xff0001fe, 0xff0101fe, 0x00000100},
        {0x00010000, 0xff0101fe, 0xff0101ff, 0xff0101fe, 0x00010000,
         0x00000100, 0x00000100, 0x00000101, 0xff0100fe, 0x00010100,
         0x00000001, 0x00010000, 0x00010001, 0xff0100fe, 0x00000001,
         0x00010000, 0x00000000, 0x00000000, 0xff0101ff, 0x00000001,
         0x00010101, 0x00010000, 0x00010001, 0xff0101ff, 0x00000100},
        {0x00010001, 0xff0100fe, 0xff0101fe, 0xff0101fe, 0x00000000,
         0x00000001, 0xff0101ff, 0x00000000, 0xff0000fe, 0x00010000,
         0x00010001, 0xff0100ff, 0xff0001ff, 0xff0101fe, 0x00000000,
         0x00010100, 0xff0100fe, 0x00000101, 0xff0100ff, 0x00010000,
         0x00000000, 0xff0000fe, 0xff0100ff, 0xff0100fe, 0x00000000},
        {0x00000100, 0xff0001fe, 0xff0000ff, 0xff0000ff, 0x00010101,
         0x00010001, 0xff0101ff, 0x00000100, 0xff0100ff, 0x00000100,
         0x00000000, 0xff0000ff, 0xff0001fe, 0xff0000fe, 0x00000001,
         0x00000101, 0x00010100, 0x00010101, 0xff0000fe, 0x00010100,
         0x00000001, 0xff0100ff, 0xff0100ff, 0xff0101ff, 0x00000101}};

    // Matriz temporária para armazenar os valores RGB
    int rgb_matrix[MATRIX_ROWS][MATRIX_COLS][MATRIX_DEPTH];

    // Processar cada matriz e exibir o resultado
    for (int matrix_index = 0; matrix_index < NUMERO_DE_IMAGENS; matrix_index++)
    {
        printf("Matriz %d:\n", matrix_index + 1);
        processMatrix(new_piskel_data[matrix_index], rgb_matrix);
        printMatrix(rgb_matrix);
        printf("\n");
    }

    return 0;
}
