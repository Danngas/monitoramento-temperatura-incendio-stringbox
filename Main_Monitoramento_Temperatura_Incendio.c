#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "lib/ssd1306.h"
#include "lib/font.h"
#include "numeros.h"
#include "pico/bootrom.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C
#define JOYSTICK_X_PIN 26 // GPIO para eixo X
#define JOYSTICK_Y_PIN 27 // GPIO para eixo Y
#define JOYSTICK_PB 22    // GPIO para botão do Joystick

#define LED_R 11 // GPIO do LED vermelho
#define LED_G 12 // GPIO do LED verde
#define LED_B 13 // GPIO do LED azul

#define Pino_BOTAO_A 5   // Botão A
#define Pino_BOTAO_B 6   // Botão B (simula sensor de fogo)
#define MATRIZ_LED_PIN 7 // Pino da matriz de LEDs

#define BUZZER_PIN 21
float divisor_frequency = 125; // Divisor de frequência para ajustar o tom do PWM

int countdown = 9;

volatile bool toggle_green_led = false;
volatile bool toggle_leds = true;
volatile uint8_t border_style = 1;

// Variáveis globais para debouncing
static absolute_time_t last_button_a_time = 0;
static absolute_time_t last_button_b_time = 0;
static absolute_time_t last_joystick_pb_time = 0;
const uint64_t DEBOUNCE_TIME = 200000; // 200ms em microssegundos

// Declaração da função pwm_set_duty
void pwm_set_duty(uint gpio, uint16_t value);

// Declarações de funções
void set_led_matrix_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);
void set_rgb_led(uint8_t r, uint8_t g, uint8_t b);
float read_temperature(float adc_x);
bool read_fire_sensor(void);
void update_display(void);
void handle_button_1(void);
void handle_button_2(void);
void handle_button_3(void);
void handle_button_4(void);
void show_debug_screen(uint16_t adc_x, uint16_t adc_y, float temp, bool fire_detected);

// Definições de estados do sistema
typedef enum
{
    SYSTEM_NORMAL,    // 0-39°C
    SYSTEM_ATTENTION, // 40-59°C
    SYSTEM_CRITICAL   // 60°C ou mais
} SystemState;

// Estrutura para status do sistema
typedef struct
{
    SystemState state;
    float current_temp;
    bool fire_detected;
} SystemStatus;

// Variável global do status do sistema
SystemStatus system_status = {
    .state = SYSTEM_NORMAL,
    .current_temp = 0.0f,
    .fire_detected = false};

void buzzer_alerta_incendio()
{
    for (int i = 0; i < 5; i++)
    {
        gpio_put(BUZZER_PIN, 1); // Liga o buzzer
        sleep_ms(5);             // Som por 100 ms
        gpio_put(BUZZER_PIN, 0); // Desliga o buzzer
        sleep_ms(10);            // Pausa entre os bipes
    }
}

// Função para atualizar o estado do sistema

// Função para exibir tela de depuração
void show_debug_screen(uint16_t adc_x, uint16_t adc_y, float temp, bool fire_detected)
{
    // Limpa o terminal e move o cursor para o início
    printf("\033[2J\033[H");

    printf("===== MONITORAMENTO DE TEMPERATURA E INCÊNDIO =====\n");
    printf("Temperatura Atual:       %.1f °C\n", temp);
    printf("Temperatura de Referência:  0.0 °C\n");
    printf("Sensor de Incêndio:      %s\n", fire_detected ? "DETECTADO" : "NORMAL");

    // Status do sistema baseado na temperatura
    if (temp >= 60.0f)
    {
        printf("Estado do Sistema:       CRÍTICO\n");
        printf("Risco de Incêndio:       ALTO\n");
        printf("Ação Recomendada:        Desligar String Box\n");
        system_status.state = SYSTEM_CRITICAL;
        printf("ALERTA: Temperatura Crítica! %.1f°C\n", system_status.current_temp);
    }
    else if (temp >= 40.0f)
    {
        printf("Estado do Sistema:       ATENÇÃO\n");
        printf("Risco de Incêndio:       BAIXO\n");
        printf("Ação Recomendada:        Monitorar (String Box Energizada)\n");
        system_status.state = SYSTEM_ATTENTION;
        printf("Atenção: Temperatura elevada! %.1f°C\n", system_status.current_temp);
    }
    else
    {
        printf("Estado do Sistema:       NORMAL\n");
        printf("Risco de Incêndio:       NULO\n");
        printf("Ação Recomendada:        Operação Segura\n");
        system_status.state = SYSTEM_NORMAL;
        printf("Temperatura normal: %.1f°C\n", system_status.current_temp);
    }

    printf("\nJoystick:\n");
    printf("  X = %4d   |   Y = %4d   |   Clicado: %s\n", adc_x, adc_y, gpio_get(JOYSTICK_PB) ? "NÃO" : "SIM");

    // Status do LED RGB
    if (temp >= 60.0f)
    {
        printf("\nLED RGB:     VERMELHO (Sistema Desligado)\n");
    }
    else
    {
        printf("\nLED RGB:     VERDE (Sistema Ligado)\n");
    }

    printf("===================================================\n");
}

// Função para atualizar a matriz de LEDs
void update_led_matrix(void)
{
    static uint32_t last_update = 0;

    if (system_status.current_temp >= 60.0f || system_status.fire_detected)
    {
        set_rgb_led(0, 0, 255);
        buzzer_alerta_incendio();

        if (countdown <= 0)
        {
            vermelho();
            countdown = 0;
        }
        else
        {
            Num(countdown);
            countdown--;
        }
    }
    else if (system_status.current_temp >= 40.0f)
    {
        amarelo();
    }
    else
    {
        verde();
        set_rgb_led(255, 0, 0); // RGB verde
        countdown = 9;
    }
}

// Implementação da função
void pwm_set_duty(uint gpio, uint16_t value)
{
    pwm_set_gpio_level(gpio, value);
}

// Função de callback para interrupções dos botões
void button_callback(uint gpio, uint32_t events)
{
    absolute_time_t now = get_absolute_time();
    uint64_t time_diff;

    switch (gpio)
    {
    case Pino_BOTAO_A:
        time_diff = absolute_time_diff_us(last_button_a_time, now);
        if (time_diff >= DEBOUNCE_TIME)
        {
            last_button_a_time = now;
            // Ação para bOTAO a
            reset_usb_boot(0, 0);
        }
        break;

    case Pino_BOTAO_B:
        time_diff = absolute_time_diff_us(last_button_b_time, now);
        if (time_diff >= DEBOUNCE_TIME)
        {
            last_button_b_time = now;
            // Ação para botão B
            system_status.fire_detected = !system_status.fire_detected;
        }
        break;

    case JOYSTICK_PB:
        time_diff = absolute_time_diff_us(last_joystick_pb_time, now);
        if (time_diff >= DEBOUNCE_TIME)
        {
            last_joystick_pb_time = now;
            // Ação para botão do joystick
            toggle_leds = !toggle_leds;
            if (!toggle_leds)
            {
                pwm_set_duty(LED_R, 0);
                pwm_set_duty(LED_B, 0);
            }
        }
        break;
    }
}

// Implementação das funções
float read_temperature(float adc_x)
{
    // Converte valor ADC para temperatura (0-100°C)
    return (float)(adc_x * 100.0f / 4095.0f) - 20;
}

bool read_fire_sensor(void)
{
    // Usa o botão B para simular sensor de fogo
    return !gpio_get(Pino_BOTAO_B); // Inverte porque o botão está com pull-up
}

// Função para atualizar a matriz de LEDs
void update_rgb_led(void)
{
    if (system_status.state == SYSTEM_NORMAL)
    {
        // verde();  // Liga matriz na cor verde quando em estado normal
    }
    else
    {
        printNum(); // Apaga a matriz quando fora do estado normal
    }
}

// Função para controlar o LED RGB
void set_rgb_led(uint8_t r, uint8_t g, uint8_t b)
{
    // Converte valores de 0-255 para 0-2000 (faixa do PWM)
    uint16_t duty_r = (r * 2000) / 255;
    uint16_t duty_g = (g * 2000) / 255;
    uint16_t duty_b = (b * 2000) / 255;

    // Aplica os valores ao PWM
    pwm_set_duty(LED_R, duty_r);
    pwm_set_duty(LED_G, duty_g);
    pwm_set_duty(LED_B, duty_b);
}

int main()
{
    stdio_init_all();

    // Inicializa a matriz de LEDs
    npInit(MATRIZ_LED_PIN);

    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    // Para ser utilizado o modo BOOTSEL com botão B
    gpio_init(Pino_BOTAO_B);
    gpio_set_dir(Pino_BOTAO_B, GPIO_IN);
    gpio_pull_up(Pino_BOTAO_B);
    gpio_set_irq_enabled_with_callback(Pino_BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &button_callback);

    gpio_init(JOYSTICK_PB);
    gpio_set_dir(JOYSTICK_PB, GPIO_IN);
    gpio_pull_up(JOYSTICK_PB);

    gpio_init(Pino_BOTAO_A);
    gpio_set_dir(Pino_BOTAO_A, GPIO_IN);
    gpio_pull_up(Pino_BOTAO_A);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);                    // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA);                                        // Pull up the data line
    gpio_pull_up(I2C_SCL);                                        // Pull up the clock line
    ssd1306_t ssd;                                                // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd);                                         // Configura o display
    ssd1306_send_data(&ssd);                                      // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

    int16_t x_pos;
    int16_t y_pos;
    char str_x[5];
    char str_y[5];

    const uint16_t CENTRO_X = 1870;
    const uint16_t CENTRO_Y = 1969;
    const uint16_t ZONA_MORTA = 200;
    const int VELOCIDADE_MAX = 5;

    // Posição central do display
    const int CENTRO_DISPLAY_X = 64;
    const int CENTRO_DISPLAY_Y = 32;

    // Posição inicial da letra A (centro do display)
    int pos_x = CENTRO_DISPLAY_X;
    int pos_y = CENTRO_DISPLAY_Y;
    const uint8_t QUADRADO_SIZE = 8; // Você pode alterar este valor para mudar o tamanho
                                     // Cálculo das margens baseadas no tamanho do quadrado
    const uint8_t MARGEM = 2;        // Espaço mínimo entre o quadrado e a borda
    const uint8_t LIMITE_X_MIN = MARGEM;
    const uint8_t LIMITE_X_MAX = WIDTH - QUADRADO_SIZE - MARGEM;
    const uint8_t LIMITE_Y_MIN = MARGEM;
    const uint8_t LIMITE_Y_MAX = HEIGHT - QUADRADO_SIZE - MARGEM;
    bool cor = true;

    // Configura os LEDs para modo PWM
    gpio_set_function(LED_R, GPIO_FUNC_PWM);
    gpio_set_function(LED_G, GPIO_FUNC_PWM);
    gpio_set_function(LED_B, GPIO_FUNC_PWM);

    // Configura os botões
    gpio_set_irq_enabled_with_callback(Pino_BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &button_callback);
    gpio_set_irq_enabled_with_callback(Pino_BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &button_callback);
    gpio_set_irq_enabled_with_callback(JOYSTICK_PB, GPIO_IRQ_EDGE_FALL, true, &button_callback);

    // Configuração do PWM para os LEDs
    uint slice_num_r = pwm_gpio_to_slice_num(LED_R);
    uint slice_num_g = pwm_gpio_to_slice_num(LED_G);
    uint slice_num_b = pwm_gpio_to_slice_num(LED_B);

    // Configura PWM 50Hz para todos os LEDs
    pwm_set_clkdiv(slice_num_r, 1250);
    pwm_set_clkdiv(slice_num_g, 1250);
    pwm_set_clkdiv(slice_num_b, 1250);

    pwm_set_wrap(slice_num_r, 2000);
    pwm_set_wrap(slice_num_g, 2000);
    pwm_set_wrap(slice_num_b, 2000);

    // Habilita o PWM para todos os LEDs
    pwm_set_enabled(slice_num_r, true);
    pwm_set_enabled(slice_num_g, true);
    pwm_set_enabled(slice_num_b, true);

    DesligaMatriz();

    uint64_t ultimo_tempo = 0;
    const uint64_t intervalo = 1000 * 1000; // 200 ms em microssegundos

    while (true)
    {
        // Lê valores do joystick
        adc_select_input(0); // Canal 0 para joystick X
        uint16_t adc_x = adc_read();
        adc_select_input(1); // Canal 1 para joystick Y
        uint16_t adc_y = adc_read();

        // Lê temperatura e estado do sensor
        float temp = read_temperature(adc_x);
        system_status.current_temp = temp;
        // bool fire = read_fire_sensor();
        // system_status.fire_detected = fire;
        //  Exibe tela de depuração
        uint64_t agora = time_us_64(); // tempo atual em microssegundos

        if (agora - ultimo_tempo >= intervalo)
        {
            ultimo_tempo = agora;
            show_debug_screen(adc_x, adc_y, temp, system_status.fire_detected);
        }

        // Atualiza posição do quadrado com limites
        x_pos = ((adc_y * (WIDTH - 24)) / 4095) + 8;            // Adiciona margem horizontal
        y_pos = HEIGHT - 16 - ((adc_x * (HEIGHT - 24)) / 4095); // Adiciona margem vertical

        // Garante que o quadrado fique dentro dos limites
        if (x_pos < 8)
            x_pos = 8; // Limite esquerdo
        if (x_pos > WIDTH - 16)
            x_pos = WIDTH - 16; // Limite direito
        if (y_pos < 8)
            y_pos = 8; // Limite superior
        if (y_pos > HEIGHT - 16)
            y_pos = HEIGHT - 16; // Limite inferior

        // Atualiza display
        ssd1306_fill(&ssd, false);
        ssd1306_rect(&ssd, y_pos, x_pos, QUADRADO_SIZE, QUADRADO_SIZE, true, true);

        // Desenha borda com espessura variável
        if (border_style == 1)
        {
            // Borda fina (1 pixel)
            ssd1306_rect(&ssd, 0, 0, WIDTH, HEIGHT, true, false);
        }
        else
        {
            // Borda grossa (2 pixels)
            ssd1306_rect(&ssd, 0, 0, WIDTH, HEIGHT, true, false);
            ssd1306_rect(&ssd, 1, 1, WIDTH - 2, HEIGHT - 2, true, false);
        }

        // Converte valores ADC para string e exibe
        sprintf(str_x, "%d", adc_x);
        sprintf(str_y, "%d", adc_y);
        // ssd1306_draw_string(&ssd, str_x, 8, 52);
        // ssd1306_draw_string(&ssd, str_y, 49, 52);

        ssd1306_send_data(&ssd);

        // Controle dos LEDs PWM (corrigido vermelho para X e azul para Y)
        if (toggle_leds)
        {
            int32_t dist_x = abs(2048 - adc_x); // Distância do centro para eixo X (LED vermelho)
            int32_t dist_y = abs(2048 - adc_y); // Distância do centro para eixo Y (LED azul)
            const int32_t deadzone = 300;

            // LED Vermelho controlado pelo eixo X
            uint16_t duty_r = 0;
            if (dist_y > deadzone)
            {
                duty_r = ((dist_y - deadzone) * 2000) / (2048 - deadzone);
                if (duty_r > 2000)
                    duty_r = 2000;
            }

            // LED Azul controlado pelo eixo Y
            uint16_t duty_b = 0;
            if (dist_x > deadzone)
            {
                duty_b = ((dist_x - deadzone) * 2000) / (2048 - deadzone);
                if (duty_b > 2000)
                    duty_b = 2000;
            }

            //  pwm_set_duty(LED_R, duty_r); // LED vermelho com eixo X
            //  pwm_set_duty(LED_B, duty_b); // LED azul com eixo Y
        }

        // Atualiza estado do sistema
        // printf("Temperatura lida: %.1f°C\n", temp);

        // Atualiza matriz de LEDs
        update_led_matrix();

        // Atualiza LED RGB
        // update_rgb_led();

        sleep_ms(100); // Atualiza a cada 100ms
    }

    // Desliga a matriz de LEDs antes de encerrar
    DesligaMatriz();
    return 0;
}