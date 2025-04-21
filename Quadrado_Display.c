#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "lib/ssd1306.h"
#include "lib/font.h"

// Definições do display
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64
#define SQUARE_SIZE 8
#define I2C_PORT i2c0
#define I2C_SDA 20
#define I2C_SCL 21
#define DISPLAY_ADDR 0x3C

// Definições do joystick
#define JOYSTICK_X_PIN 26
#define JOYSTICK_Y_PIN 27
#define JOYSTICK_BUTTON 22

// Definições dos botões
#define BUTTON_A 5
#define BUTTON_B 6

// Definições dos LEDs RGB
#define LED_R 13
#define LED_G 11
#define LED_B 12

// Estrutura global do display
static ssd1306_t display;

// Função para inicializar o PWM
void init_pwm(void) {
    // Configura os pinos dos LEDs para PWM
    gpio_set_function(LED_R, GPIO_FUNC_PWM);
    gpio_set_function(LED_G, GPIO_FUNC_PWM);
    gpio_set_function(LED_B, GPIO_FUNC_PWM);

    // Configura os slices de PWM
    uint slice_num_r = pwm_gpio_to_slice_num(LED_R);
    uint slice_num_g = pwm_gpio_to_slice_num(LED_G);
    uint slice_num_b = pwm_gpio_to_slice_num(LED_B);

    // Configura PWM para 50Hz
    pwm_config config = pwm_get_default_config();
    pwm_config_set_clkdiv(&config, 1250);
    pwm_config_set_wrap(&config, 2000);

    // Aplica a configuração para todos os slices
    pwm_init(slice_num_r, &config, true);
    pwm_init(slice_num_g, &config, true);
    pwm_init(slice_num_b, &config, true);
}

// Função para controlar o LED RGB
void set_rgb_led(uint8_t r, uint8_t g, uint8_t b) {
    pwm_set_gpio_level(LED_R, r * 8);  // Multiplica por 8 para converter 0-255 para 0-2040
    pwm_set_gpio_level(LED_G, g * 8);
    pwm_set_gpio_level(LED_B, b * 8);
}

// Função para atualizar o display com o quadrado
void update_display(void) {
    // Lê valores do joystick
    adc_select_input(0); // Eixo X
    uint16_t adc_x = adc_read();
    adc_select_input(1); // Eixo Y
    uint16_t adc_y = adc_read();

    // Converte valores ADC para posições no display
    int x_pos = ((adc_y * (DISPLAY_WIDTH - SQUARE_SIZE)) / 4095);
    int y_pos = ((adc_x * (DISPLAY_HEIGHT - SQUARE_SIZE)) / 4095);

    // Garante que o quadrado fique dentro dos limites
    if (x_pos < 0) x_pos = 0;
    if (x_pos > DISPLAY_WIDTH - SQUARE_SIZE) x_pos = DISPLAY_WIDTH - SQUARE_SIZE;
    if (y_pos < 0) y_pos = 0;
    if (y_pos > DISPLAY_HEIGHT - SQUARE_SIZE) y_pos = DISPLAY_HEIGHT - SQUARE_SIZE;

    // Atualiza display
    ssd1306_fill(&display, false);
    ssd1306_rect(&display, x_pos, y_pos, SQUARE_SIZE, SQUARE_SIZE, true, true);
    ssd1306_send_data(&display);
}

int main() {
    stdio_init_all();
    printf("Inicializando sistema...\n");

    // Inicializa I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa display
    ssd1306_init(&display, DISPLAY_WIDTH, DISPLAY_HEIGHT, false, DISPLAY_ADDR, I2C_PORT);
    ssd1306_config(&display);
    ssd1306_fill(&display, false);
    ssd1306_send_data(&display);

    // Inicializa ADC
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

    // Inicializa PWM
    init_pwm();

    // Configura botões
    gpio_init(BUTTON_A);
    gpio_init(BUTTON_B);
    gpio_init(JOYSTICK_BUTTON);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_set_dir(JOYSTICK_BUTTON, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_pull_up(BUTTON_B);
    gpio_pull_up(JOYSTICK_BUTTON);

    printf("Sistema inicializado. Movimente o joystick para controlar o quadrado.\n");

    // Loop principal
    while (true) {
        update_display();
        sleep_ms(20); // 50Hz de atualização
    }

    return 0;
}