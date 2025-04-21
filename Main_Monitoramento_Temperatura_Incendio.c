#include <stdio.h>              // Biblioteca padrão para entrada e saída (ex: printf)
#include <stdlib.h>             // Biblioteca padrão para funções utilitárias (ex: malloc, atoi)

#include "pico/stdlib.h"        // Biblioteca principal do SDK do Raspberry Pi Pico (GPIO, delays, etc.)
#include "hardware/adc.h"       // Controla o ADC interno do Pico (leitura de sinais analógicos)
#include "hardware/i2c.h"       // Permite comunicação I2C (ex: com display OLED)
#include "hardware/pwm.h"       // Permite controle de PWM (ex: brilho de LEDs RGB)

#include "lib/ssd1306.h"        // Biblioteca para controle do display OLED SSD1306 via I2C
#include "lib/font.h"           // Biblioteca auxiliar de fontes para uso com o display OLED
#include "numeros.h"            // Biblioteca com funções para exibir números na matriz de LEDs

#include "pico/bootrom.h"       // Usada para acessar funções especiais da ROM, como reinício via USB (modo BOOTSEL)


/// ===============================
// === DEFINIÇÕES DE CONSTANTES ===
// ===============================

// --- Configurações do barramento I2C (usado pelo display OLED) ---
#define I2C_PORT i2c1           // Utiliza o controlador I2C 1
#define I2C_SDA 14              // Pino GPIO 14 como linha de dados I2C (SDA)
#define I2C_SCL 15              // Pino GPIO 15 como linha de clock I2C (SCL)
#define endereco 0x3C           // Endereço I2C do display OLED SSD1306

// --- Pinos do Joystick ---
#define JOYSTICK_X_PIN 26       // Pino GPIO 26 para leitura do eixo X (ADC0)
#define JOYSTICK_Y_PIN 27       // Pino GPIO 27 para leitura do eixo Y (ADC1)

// --- LEDs RGB (controle de status) ---
#define LED_R 11                // Pino GPIO 11 para canal vermelho do LED RGB
#define LED_G 12                // Pino GPIO 12 para canal verde do LED RGB
#define LED_B 13                // Pino GPIO 13 para canal azul do LED RGB

// --- Botões físicos no sistema ---
#define Pino_BOTAO_A 5          // Pino GPIO 5 para o botão A (ex: reinício em modo BOOTSEL)
#define Pino_BOTAO_B 6          // Pino GPIO 6 para o botão B (simula sensor de incêndio)
#define MATRIZ_LED_PIN 7        // Pino GPIO 7 conectado à matriz de LEDs WS2812

// --- Buzzer (alerta sonoro) ---
#define BUZZER_PIN 21           // Pino GPIO 21 usado para ativar o buzzer (alarme)

float divisor_frequency = 125;  // Divisor de frequência usado para ajustar o tom do PWM do buzzer
int countdown = 9;              // Contador para desligamento em caso de incêndio crítico

volatile bool toggle_green_led = false;  // Controle de piscada do LED verde (não usado no trecho atual)
volatile bool toggle_leds = true;        // Flag que permite ativar/desativar o controle de LEDs via joystick
volatile uint8_t border_style = 1;       // Estilo da borda no display OLED (1 = fina, 2 = grossa)

// Controle de tempo para evitar múltiplos acionamentos indevidos (debounce)
static absolute_time_t last_button_a_time = 0;        // Último tempo de acionamento do botão A
static absolute_time_t last_button_b_time = 0;        // Último tempo de acionamento do botão B

const uint64_t DEBOUNCE_TIME = 200000;  // Tempo mínimo entre cliques: 200ms (em microssegundos)

// ===============================
// === VARIÁVEL DE ESTADO DO SISTEMA ===
// ===============================

// Enum que define os estados possíveis do sistema com base na temperatura
typedef enum
{
    SYSTEM_NORMAL,    // Temperatura segura (< 40°C)
    SYSTEM_ATTENTION, // Temperatura elevada (40–59°C)
    SYSTEM_CRITICAL   // Temperatura crítica (≥ 60°C)
} SystemState;


typedef struct
{
    SystemState state;       // Estado atual (NORMAL, ATENÇÃO ou CRÍTICO)
    float current_temp;      // Temperatura atual lida pelo sensor
    bool fire_detected;      // Status do sensor de incêndio (true = fogo detectado)
} SystemStatus;

SystemStatus system_status = {
    .state = SYSTEM_NORMAL,       // Inicializa como sistema normal
    .current_temp = 0.0f,         // Temperatura inicial
    .fire_detected = false        // Nenhum incêndio detectado ao iniciar
};

int relatorio = 0;  // Flag que indica se o relatório de evento já foi gerado (evita repetição)


// ===============================
// === PROTÓTIPOS DE FUNÇÕES ===
// ===============================

// --- PWM ---
// Define o nível do sinal PWM em um pino específico (usado para controle de brilho dos LEDs)
void pwm_set_duty(uint gpio, uint16_t value);

// --- LED e Display ---
// Define a cor de um pixel na matriz de LEDs WS2812
void set_led_matrix_pixel(int x, int y, uint8_t r, uint8_t g, uint8_t b);

// Controla a cor do LED RGB com base nos valores de vermelho, verde e azul (0–255)
void set_rgb_led(uint8_t r, uint8_t g, uint8_t b);

// Atualiza a matriz de LEDs com base no estado do sistema (verde, amarelo, vermelho)
void update_led_matrix(void);

// Atualiza o estado da matriz dependendo do nível de alerta (usado para desligar se necessário)
void update_rgb_led(void);

// (Prototipada mas não implementada) — usada para atualização do display OLED
void update_display(void);

// --- Temperatura e Sensores ---
// Converte o valor lido do ADC em temperatura (°C)
float read_temperature(float adc_x);

// Retorna o status do sensor de incêndio (simulado via botão B)
bool read_fire_sensor(void);

// --- Interações e eventos ---
// Callback das interrupções de botões (A, B e botão do joystick)
void button_callback(uint gpio, uint32_t events);

// (Reservadas para ações futuras com botões ou joystick)
void handle_button_1(void); // Não implementada
void handle_button_2(void); // Não implementada
void handle_button_3(void); // Não implementada
void handle_button_4(void); // Não implementada

// --- Depuração e Relatório ---
// Exibe as informações do sistema no terminal (temperatura, estado, joystick, LED)
void show_debug_screen(uint16_t adc_x, uint16_t adc_y, float temp, bool fire_detected);

// Gera um relatório formatado no terminal quando incêndio ou temperatura crítica é detectado
void gerar_relatorio_evento(SystemStatus status);

// --- Buzzer ---
// Emite sinal sonoro intermitente no buzzer (5 bipes rápidos)
void buzzer_alerta_incendio(void);

int main(void)
{
    // Inicializa comunicação serial padrão (UART via USB) para printf
    stdio_init_all();

    // Inicializa a matriz de LEDs WS2812 conectada ao pino definido
    npInit(MATRIZ_LED_PIN);

    // Configura o pino do buzzer como saída
    gpio_init(BUZZER_PIN);
    gpio_set_dir(BUZZER_PIN, GPIO_OUT);

    // --- Configuração dos botões físicos ---

    // Botão B (simula sensor de fogo)
    gpio_init(Pino_BOTAO_B);
    gpio_set_dir(Pino_BOTAO_B, GPIO_IN);
    gpio_pull_up(Pino_BOTAO_B);
    gpio_set_irq_enabled_with_callback(Pino_BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &button_callback);



    // Botão A (modo BOOTSEL)
    gpio_init(Pino_BOTAO_A);
    gpio_set_dir(Pino_BOTAO_A, GPIO_IN);
    gpio_pull_up(Pino_BOTAO_A);

    // --- Inicializa barramento I2C e configura display OLED SSD1306 ---

    i2c_init(I2C_PORT, 400 * 1000); // Inicializa I2C a 400kHz
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa estrutura e configura o display OLED
    ssd1306_t ssd;
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // --- Inicializa ADC para ler joystick analógico ---
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

    // --- Variáveis de controle da posição e exibição no display ---
    int16_t x_pos, y_pos;
    char str_x[5], str_y[5];

    const uint16_t CENTRO_X = 1870;
    const uint16_t CENTRO_Y = 1969;
    const uint16_t ZONA_MORTA = 200;
    const int VELOCIDADE_MAX = 5;

    const int CENTRO_DISPLAY_X = 64;
    const int CENTRO_DISPLAY_Y = 32;

    int pos_x = CENTRO_DISPLAY_X;
    int pos_y = CENTRO_DISPLAY_Y;

    const uint8_t QUADRADO_SIZE = 8;
    const uint8_t MARGEM = 2;
    const uint8_t LIMITE_X_MIN = MARGEM;
    const uint8_t LIMITE_X_MAX = WIDTH - QUADRADO_SIZE - MARGEM;
    const uint8_t LIMITE_Y_MIN = MARGEM;
    const uint8_t LIMITE_Y_MAX = HEIGHT - QUADRADO_SIZE - MARGEM;

    bool cor = true;

    // --- Configura LED RGB como saída PWM ---
    gpio_set_function(LED_R, GPIO_FUNC_PWM);
    gpio_set_function(LED_G, GPIO_FUNC_PWM);
    gpio_set_function(LED_B, GPIO_FUNC_PWM);

    // Ativa interrupções para botões e joystick
    gpio_set_irq_enabled_with_callback(Pino_BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &button_callback);
    gpio_set_irq_enabled_with_callback(Pino_BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &button_callback);

    // --- Configura PWM dos LEDs RGB ---
    uint slice_num_r = pwm_gpio_to_slice_num(LED_R);
    uint slice_num_g = pwm_gpio_to_slice_num(LED_G);
    uint slice_num_b = pwm_gpio_to_slice_num(LED_B);

    // Frequência ~50Hz
    pwm_set_clkdiv(slice_num_r, 1250);
    pwm_set_clkdiv(slice_num_g, 1250);
    pwm_set_clkdiv(slice_num_b, 1250);

    pwm_set_wrap(slice_num_r, 2000);
    pwm_set_wrap(slice_num_g, 2000);
    pwm_set_wrap(slice_num_b, 2000);

    pwm_set_enabled(slice_num_r, true);
    pwm_set_enabled(slice_num_g, true);
    pwm_set_enabled(slice_num_b, true);

    // Desliga a matriz de LEDs no início
    DesligaMatriz();

    // Controle de tempo para atualizar a tela periodicamente
    uint64_t ultimo_tempo = 0;
    const uint64_t intervalo = 1000 * 1000; // Atualiza a cada 1 segundo

    // ===============================
    // === LOOP PRINCIPAL ============
    // ===============================
    while (true)
    {
        // --- Leitura do joystick (X e Y analógicos via ADC) ---
        adc_select_input(0);
        uint16_t adc_x = adc_read();
        adc_select_input(1);
        uint16_t adc_y = adc_read();

        // Atualiza temperatura com base na leitura do ADC
        float temp = read_temperature(adc_x);
        system_status.current_temp = temp;

        // Atualiza a tela OLED a cada segundo
        uint64_t agora = time_us_64();
        if (agora - ultimo_tempo >= intervalo)
        {
            ultimo_tempo = agora;
            show_debug_screen(adc_x, adc_y, temp, system_status.fire_detected);
        }

        // --- Calcula posição do quadrado na tela com base no joystick ---
        x_pos = ((adc_y * (WIDTH - 24)) / 4095) + 8;
        y_pos = HEIGHT - 16 - ((adc_x * (HEIGHT - 24)) / 4095);

        // Limita a posição para manter dentro da tela
        if (x_pos < 8) x_pos = 8;
        if (x_pos > WIDTH - 16) x_pos = WIDTH - 16;
        if (y_pos < 8) y_pos = 8;
        if (y_pos > HEIGHT - 16) y_pos = HEIGHT - 16;

        // --- Atualiza o display OLED com o quadrado e bordas ---
        ssd1306_fill(&ssd, false);
        ssd1306_rect(&ssd, y_pos, x_pos, QUADRADO_SIZE, QUADRADO_SIZE, true, true);

        // Desenha borda (fina ou grossa) de acordo com o estilo selecionado
        if (border_style == 1)
        {
            ssd1306_rect(&ssd, 0, 0, WIDTH, HEIGHT, true, false); // borda 1px
        }
        else
        {
            ssd1306_rect(&ssd, 0, 0, WIDTH, HEIGHT, true, false);       // camada externa
            ssd1306_rect(&ssd, 1, 1, WIDTH - 2, HEIGHT - 2, true, false); // camada interna
        }

        // Atualiza display com os dados
        ssd1306_send_data(&ssd);

        // --- Controle do brilho dos LEDs RGB com base no joystick ---
        if (toggle_leds)
        {
            int32_t dist_x = abs(2048 - adc_x);
            int32_t dist_y = abs(2048 - adc_y);
            const int32_t deadzone = 300;

            uint16_t duty_r = 0;
            if (dist_y > deadzone)
            {
                duty_r = ((dist_y - deadzone) * 2000) / (2048 - deadzone);
                if (duty_r > 2000) duty_r = 2000;
            }

            uint16_t duty_b = 0;
            if (dist_x > deadzone)
            {
                duty_b = ((dist_x - deadzone) * 2000) / (2048 - deadzone);
                if (duty_b > 2000) duty_b = 2000;
            }

            // pwm_set_duty(LED_R, duty_r);
            // pwm_set_duty(LED_B, duty_b);
        }

        // Atualiza status da matriz de LEDs conforme o estado do sistema
        update_led_matrix();

        // Delay entre cada iteração do loop
        sleep_ms(100);
    }

    // Desliga a matriz de LEDs ao encerrar o programa
    DesligaMatriz();
    return 0;
}


// FUNCOES PARA FUNCIOANAMENTO DO PROGRAMA

// ================================================
// === CONTROLE DE PWM PARA LEDs RGB ==============
// ================================================
void pwm_set_duty(uint gpio, uint16_t value)
{
    pwm_set_gpio_level(gpio, value);
}

// ================================================
// === CONTROLE DO LED RGB COM PWM ================
// ================================================
void set_rgb_led(uint8_t r, uint8_t g, uint8_t b)
{
    uint16_t duty_r = (r * 2000) / 255;
    uint16_t duty_g = (g * 2000) / 255;
    uint16_t duty_b = (b * 2000) / 255;

    pwm_set_duty(LED_R, duty_r);
    pwm_set_duty(LED_G, duty_g);
    pwm_set_duty(LED_B, duty_b);
}

// ================================================
// === LEITURA DE TEMPERATURA PELO ADC ============
// ================================================
float read_temperature(float adc_x)
{
    return (float)(adc_x * 100.0f / 4095.0f) - 20;
}

// ================================================
// === LEITURA DO SENSOR DE INCÊNDIO (simulado) ===
// ================================================
bool read_fire_sensor(void)
{
    return !gpio_get(Pino_BOTAO_B);
}

// ================================================
// === CALLBACK PARA INTERRUPÇÕES DOS BOTÕES ======
// ================================================
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
            reset_usb_boot(0, 0); // Reinicia em modo BOOTSEL
        }
        break;

    case Pino_BOTAO_B:
        time_diff = absolute_time_diff_us(last_button_b_time, now);
        if (time_diff >= DEBOUNCE_TIME)
        {
            last_button_b_time = now;
            system_status.fire_detected = !system_status.fire_detected;
        }
        break;

  
    
    }
}

// ================================================
// === ALERTA SONORO COM O BUZZER ==================
// ================================================
void buzzer_alerta_incendio()
{
    for (int i = 0; i < 50; i++)
    {
        gpio_put(BUZZER_PIN, 1);
        sleep_us(500);
        gpio_put(BUZZER_PIN, 0);
        sleep_us(100);
    }
}

// ================================================
// === RELATÓRIO DE EVENTOS CRÍTICOS ==============
// ================================================
void gerar_relatorio_evento(SystemStatus status)
{
    printf("\n=========== RELATÓRIO DE DESLIGAMENTO ===========\n");
    printf("Temperatura atual     : %.1f °C\n", status.current_temp);
    printf("Sensor de Incêndio    : %s\n", status.fire_detected ? "DETECTADO" : "NORMAL");

    const char *causa = "";
    if (status.fire_detected && status.current_temp >= 60.0f)
        causa = "Incêndio detectado + Temperatura Crítica";
    else if (status.fire_detected)
        causa = "Incêndio detectado";
    else if (status.current_temp >= 60.0f)
        causa = "Temperatura Crítica";
    else
        causa = "Desconhecida (falha no sistema?)";

    printf("Causa do Desligamento : %s\n", causa);
    printf("Ação Executada        : Contagem regressiva (9 a 0), Seccionamento da String Box\n");
    printf("Status Final          : SISTEMA DESENERGIZADO \n");
    printf("=================================================\n\n");
}

// ================================================
// === TELA DE DEPURAÇÃO ==========================
// ================================================
void show_debug_screen(uint16_t adc_x, uint16_t adc_y, float temp, bool fire_detected)
{
    printf("\033[2J\033[H");
    printf("===== MONITORAMENTO DE TEMPERATURA E INCÊNDIO =====\n");
    printf("Temperatura Atual:       %.1f °C\n", temp);
    printf("Temperatura de Referência:  0.0 °C\n");
    printf("Sensor de Incêndio:      %s\n", fire_detected ? "DETECTADO" : "NORMAL");

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
        printf("Ação Recomendada:        Monitorar\n");
        system_status.state = SYSTEM_ATTENTION;
        printf("Atenção: Temperatura elevada! %.1f°C\n", system_status.current_temp);
    }
    else
    {
        relatorio = 1;
        printf("Estado do Sistema:       NORMAL\n");
        printf("Risco de Incêndio:       NULO\n");
        printf("Ação Recomendada:        Operação Segura\n");
        system_status.state = SYSTEM_NORMAL;
        printf("Temperatura normal: %.1f°C\n", system_status.current_temp);
    }

    printf("\nJoystick:\n");
    printf("  X = %4d   |   Y = %4d   |",adc_x, adc_y);

    if (temp >= 60.0f)
        printf("\nLED RGB:     VERMELHO (Sistema Desligado)\n");
    else
        printf("\nLED RGB:     VERDE (Sistema Ligado)\n");

    if (temp < 40) relatorio = 0;

    printf("===================================================\n");

    if (relatorio || (temp >= 60.0f || system_status.fire_detected))
    {
        gerar_relatorio_evento(system_status);
        relatorio = 1;
    }
}

// ================================================
// === CONTROLE DA MATRIZ DE LEDS =================
// ================================================
void update_led_matrix(void)
{
    static uint32_t last_update = 0;

    if (system_status.current_temp >= 60.0f || system_status.fire_detected)
    {
        set_rgb_led(0, 0, 255); // azul
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
        set_rgb_led(255, 0, 0); // verde
        countdown = 9;
    }
}

// ================================================
// === FUNÇÃO PARA CONTROLAR O ESTADO DA MATRIZ ===
// ================================================
void update_rgb_led(void)
{
    if (system_status.state == SYSTEM_NORMAL)
    {
        // verde();
    }
    else
    {
        printNum();
    }
}
