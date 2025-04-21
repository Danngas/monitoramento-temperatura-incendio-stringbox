# Biblioteca Números (numeros.h)

## Visão Geral

A biblioteca `numeros.h` foi projetada para controlar uma matriz de LEDs RGB 5x5 utilizando um Raspberry Pi Pico. Ela fornece funções predefinidas para exibir números (0-9) e cores sólidas (vermelho, amarelo, verde) na matriz de LEDs, além de uma função para desligar o display. A biblioteca é destinada a ser incluída em outros programas para aproveitar suas funcionalidades, permitindo a exibição de números e cores de forma simples.

## Funcionalidades

- Exibe os dígitos de 0 a 9 em vermelho na matriz de LEDs 5x5.
- Exibe cores sólidas: vermelho, amarelo e verde.
- Desliga todos os LEDs da matriz.
- Intensidade ajustável para o brilho do display (padrão: 0.01).
- Interface simples para integração em outros projetos.

## Dependências

- `pico/stdlib.h` - Biblioteca padrão para o Raspberry Pi Pico.
- `matrizled.c` - Biblioteca personalizada para controle da matriz de LEDs (presume-se que fornece as funções `npWrite()`, `npClear()` e `desenhaSprite()`).
- `time.h` - Para funções de temporização, como `sleep_ms()`.

## Instalação

1. Certifique-se de que a biblioteca `matrizled.c` está disponível no diretório do seu projeto ou no caminho de inclusão.

2. Copie o arquivo `numeros.h` para o diretório do seu projeto.

3. Inclua a biblioteca no seu programa em C com:

   ```c
   #include "numeros.h"
   ```

## Uso

Para usar a biblioteca, inclua-a no seu programa e chame as funções fornecidas. A seguir, estão as funções disponíveis e suas descrições:

### Funções

- **Num(int num)**\
  Exibe um dígito (0-9) na matriz de LEDs em vermelho.

  - **Parâmetro**: `num` - Inteiro de 0 a 9.

  - **Exemplo**:

    ```c
    Num(5); // Exibe o número 5
    ```

- **DesligaMatriz()**\
  Desliga todos os LEDs da matriz.

  - **Exemplo**:

    ```c
    DesligaMatriz(); // Limpa o display
    ```

- **verde()**\
  Pisca a matriz inteira na cor verde duas vezes.

  - **Exemplo**:

    ```c
    verde(); // Exibe a cor verde duas vezes
    ```

- **amarelo()**\
  Exibe a matriz inteira na cor amarela uma vez.

  - **Exemplo**:

    ```c
    amarelo(); // Exibe a cor amarela
    ```

- **vermelho()**\
  Exibe a matriz inteira na cor vermelha uma vez.

  - **Exemplo**:

    ```c
    vermelho(); // Exibe a cor vermelha
    ```

### Exemplo de Programa

```c
#include "numeros.h"

int main() {
    // Inicializa a matriz de LEDs (gerenciado por matrizled.c)
    // Sequência de exemplo
    Num(3);          // Exibe o número 3
    sleep_ms(1000);  // Aguarda 1 segundo
    vermelho();      // Exibe a cor vermelha
    sleep_ms(1000);  // Aguarda 1 segundo
    DesligaMatriz(); // Desliga a matriz
    return 0;
}
```

## Observações

- A biblioteca assume que a matriz de LEDs é controlada pela biblioteca `matrizled.c`, que fornece funções de baixo nível como `desenhaSprite()`, `npWrite()` e `npClear()`.
- A intensidade do display é definida como 0.01 por padrão (macro `intensidade`). Modifique esse valor em `numeros.h` se desejar um brilho diferente.
- A função `sleep_ms()` é usada para temporização no código fornecido, mas está comentada em `printNum()`. Certifique-se de adicionar atrasos apropriados no seu programa para controlar o tempo de exibição.
- A biblioteca usa valores de cores RGB (por exemplo, `{255, 0, 0}` para vermelho) em um formato de array 5x5x3 para representar sprites.

## Limitações

- Suporta apenas os dígitos de 0 a 9 e três cores (vermelho, amarelo, verde).
- Depende da biblioteca `matrizled.c`, que não está incluída aqui.
- Não possui tratamento de erros para entradas inválidas na função `Num()` (valores fora de 0-9 são ignorados).

## Licença

Esta biblioteca é fornecida como está para uso educacional e pessoal. Não há garantia. Sinta-se à vontade para modificar e distribuir conforme necessário.

## Contato

Para perguntas ou contribuições, entre em contato com o mantenedor do projeto.