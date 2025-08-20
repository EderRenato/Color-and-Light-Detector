#include <stdio.h>
#include <stdlib.h>
#include <string.h> // Adicionado para usar strcpy e sprintf
#include <math.h>
#include "pico/stdlib.h"
#include "buzzer.h"
#include "gy33.h"
#include "bh1750.h"
#include "ssd1306.h"
#include "font.h"
#include "pico/bootrom.h"
#include "pico/binary_info.h"
#include "hardware/clocks.h"
#include "hardware/pio.h"
#include "ws2818b.pio.h"

// Definições gerais
#define I2C_PORT i2c0
#define SDA_PIN 0
#define SCL_PIN 1
#define RED_PIN 13
#define BLUE_PIN 12
#define GREEN_PIN 11
#define LED_COUNT 25
#define LED_PIN 7

// Display na I2C
#define I2C_PORT_DISP i2c1
#define I2C_SDA_DISP 14
#define I2C_SCL_DISP 15
#define endereco 0x3C

//Definição dos botões
#define buttonA 5  // Botão A 
#define buttonB  6  // Botão B

// Definições para matriz de LED
typedef enum {
    NIVEL_0, 
    NIVEL_1, 
    NIVEL_2, 
    NIVEL_3, 
    NIVEL_4, 
    NIVEL_5  
} LuminosityState;

// Definição da estrutura do pixel
struct pixel_t {
    uint8_t G, R, B;
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t;

// Confiugarações das máquinas de estados do PIO
npLED_t leds[LED_COUNT];
PIO np_pio;
uint sm;

// --- INÍCIO DAS VARIÁVEIS GLOBAIS ---
// Objeto do display se torna global para ser acessado por múltiplas funções
ssd1306_t ssd;

// Variáveis para armazenar os dados a serem exibidos no OLED
char g_estado_sistema[20] = "Inicializando...";
char g_cor_detectada[20] = "-";
uint8_t g_r = 0, g_g = 0, g_b = 0;
uint16_t g_lux = 0;
int digit = 0;

//tamanho real do array
#define PALETTE_SIZE (sizeof(COLOR_PALETTE)/sizeof(COLOR_PALETTE[0]))

// paleta de cores
static const uint32_t COLOR_PALETTE[] = {
    0x00550000, // Vermelho
    0x55000000, // Verde
    0x00005500, // Azul
};

static volatile uint8_t  g_color_index     = 0;
static volatile uint32_t current_color_hex = 0x00550000; // começa vermelho
volatile int state = 0; // estado inicial

// --- FIM DAS VARIÁVEIS GLOBAIS ---

// Matrizes para cada dígito 
const uint8_t states[6][5][5][3] = {
    // 0
    {
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}, 
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},    
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},    
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},    
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}  
    },
    // 1
    {
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}}
    },
    // 2
    {
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}},
        {{0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}} 
    },
    // 3
    {
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}},   
        {{0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}},
        {{0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}} 
    },
    // 4
    {
        {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
        {{0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}},
        {{0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}},
        {{0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}},
        {{0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}} 
    },
    // 5
    {
        {{0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}},
        {{0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}},
        {{0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}},   
        {{0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}},
        {{0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}, {0, 100, 0}} 
    },
};

//função da interrupção callback
void debounce(uint gpio, uint32_t events)
{
    static uint32_t last_time = 0;
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if (current_time - last_time > 200000)
    {
        last_time = current_time;

        // Botão A - controla cor dos LEDs 
        if (gpio == buttonA)
        {               
            g_color_index = (g_color_index + 1) % PALETTE_SIZE; // proximo index 
            current_color_hex = COLOR_PALETTE[g_color_index]; // seleciona a proxima cor e armazena
            printf("Botao A pressionado: cor %d = %08X\n",g_color_index,current_color_hex);
        }

        if (gpio == buttonB)
        {
            reset_usb_boot(0, 0);  // entra em modo BOOTSEL (UF2)
        }
    }
}

void buttons_init(uint button) {
    // configura pinos de entrada com pull-up
    gpio_init(button);
    gpio_set_dir(button, GPIO_IN);
    gpio_pull_up(button);
}

// Inicializa os LEDs RGB
void init_rgb() {
    // Inicializa e configura o pino VERMELHO como saída
    gpio_init(RED_PIN);
    gpio_set_dir(RED_PIN, GPIO_OUT);

    // Inicializa e configura o pino VERDE como saída
    gpio_init(GREEN_PIN);
    gpio_set_dir(GREEN_PIN, GPIO_OUT);

    // Inicializa e configura o pino AZUL como saída
    gpio_init(BLUE_PIN);
    gpio_set_dir(BLUE_PIN, GPIO_OUT);
}

// Recebe o valor em hexa e controla o RGB
void set_rgb_led_digital(uint32_t color_hex) {
    // Extrai os componentes de cor do valor hexadecimal
    uint8_t g = (color_hex >> 24) & 0xFF;
    uint8_t r = (color_hex >> 16) & 0xFF;
    uint8_t b = (color_hex >> 8)  & 0xFF;

    // Aciona os pinos. Se o valor da cor (r, g, ou b) for maior que 0,
    // a expressão se torna 'true', ligando o pino. Se for 0, se torna 'false1.
    gpio_put(RED_PIN,   r > 0);
    gpio_put(GREEN_PIN, g > 0);
    gpio_put(BLUE_PIN,  b > 0);
}

// Inicialização da matriz de LEDs
void npInit(uint pin) {
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;
    sm = pio_claim_unused_sm(np_pio, true);
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
}

// Função para configurar o LED na matriz de LEDs
void npSetLED(uint index, uint8_t r, uint8_t g, uint8_t b) {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}

// Função para escrever os dados na matriz de LEDs
void npWrite() {
    for (uint i = 0; i < LED_COUNT; i++) {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100);
}

// Função para obter o índice do LED na matriz de LEDs
int getIndex(int x, int y) {
    if (y % 2 == 0) {
        return 24 - (y * 5 + x);
    } else {
        return 24 - (y * 5 + (4 - x));
    }
}

// Função responsável por exibir o dígito na matriz de LEDs
void npDisplayDigit(int digit) {
    for (int coluna = 0; coluna < 5; coluna++) {
        for (int linha = 0; linha < 5; linha++) {
            int posicao = getIndex(linha, coluna);
            npSetLED(
                posicao,
                states[digit][coluna][linha][0],  // R
                states[digit][coluna][linha][1],  // G
                states[digit][coluna][linha][2]   // B
            );
        }
    }
    npWrite();
}

// Função para limpar a matriz de LEDs
void npClear() {
   digit = NIVEL_0;
   npDisplayDigit(digit);
}

void update_luminosity_state(uint16_t luminosity) {
    if (luminosity < 100) {
        digit = NIVEL_0;
    } 
    else if (luminosity < 1000) { // Implicitamente >= 100
        digit = NIVEL_1;
    } 
    else if (luminosity < 2000) { // Implicitamente >= 1000
        digit = NIVEL_2;
    } 
    else if (luminosity < 3000) { // Implicitamente >= 2000
        digit = NIVEL_3;
    } 
    else if (luminosity < 4000) { // Implicitamente >= 3000
        digit = NIVEL_4;
    } 
    else { // Se chegou aqui, é porque a luminosidade é >= 4000
        digit = NIVEL_5;
    }
}

void setupDisplay() {
    // I2C do Display
    i2c_init(I2C_PORT_DISP, 400 * 1000);
    gpio_set_function(I2C_SDA_DISP, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL_DISP, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA_DISP);
    gpio_pull_up(I2C_SCL_DISP);

    // Inicializa o objeto ssd global
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT_DISP);
    ssd1306_config(&ssd);

    // Limpa o display no início
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);
}

// Função de display REESCRITA para mostrar os dados das variáveis globais
void updateDisplay() {
    char buffer[30]; // Buffer para formatar as strings

    // 1. Limpa o display
    ssd1306_fill(&ssd, false);

    // 2. Exibe o Estado do Sistema
    sprintf(buffer, "Status: %s", g_estado_sistema);
    ssd1306_draw_string(&ssd, buffer, 0, 0); // Posição (x=0, y=0)

    // 3. Exibe a Cor Detectada (Texto)
    sprintf(buffer, "Cor: %s", g_cor_detectada);
    ssd1306_draw_string(&ssd, buffer, 0, 16); // Posição (x=0, y=16)

    // 4. Exibe os valores numéricos de RGB
    sprintf(buffer, "R:%d G:%d B:%d", g_r, g_g, g_b);
    ssd1306_draw_string(&ssd, buffer, 0, 32); // Posição (x=0, y=32)

    // 5. Exibe a Intensidade Luminosa
    sprintf(buffer, "Luz: %d lux", g_lux);
    ssd1306_draw_string(&ssd, buffer, 0, 48); // Posição (x=0, y=48)

    // 6. Envia os dados para o display
    ssd1306_send_data(&ssd);
}

// Função MODIFICADA para atualizar as variáveis globais
void process_and_play_tone(uint8_t r, uint8_t g, uint8_t b, uint16_t clear_val, uint16_t light) {
    const int white_threshold = 100;
    const int dominance_threshold = 30;
    const int white_tolerance = 40;

    // Atualiza as variáveis globais com os valores brutos
    g_r = r;
    g_g = g;
    g_b = b;
    g_lux = light;

    if (light < 10) {
        printf("Ambiente muito escuro\n");
        strcpy(g_estado_sistema, "Escuro"); // Atualiza estado
        strcpy(g_cor_detectada, "N/A");      // Atualiza cor
        set_buzzer_tone(BUZZER_A, C4);
        sleep_ms(250);
        set_buzzer_tone(BUZZER_A, D4);
        sleep_ms(250);
    } else {
        stop_buzzer(BUZZER_A);
        strcpy(g_estado_sistema, "Analisando"); // Atualiza estado

        if (clear_val > white_threshold && abs(r - g) < white_tolerance && abs(g - b) < white_tolerance) {
            printf("Cor detectada: Branco\n");
            strcpy(g_cor_detectada, "Branco"); // Atualiza cor
            set_buzzer_tone(BUZZER_B, D4);
            sleep_ms(250);
            set_buzzer_tone(BUZZER_B, E4);
            sleep_ms(250);
        } else if (r > g && r > b && (r - g > dominance_threshold || r - b > dominance_threshold)) {
            printf("Cor detectada: Vermelho\n");
            strcpy(g_cor_detectada, "Vermelho"); // Atualiza cor
            set_buzzer_tone(BUZZER_B, A4);
            sleep_ms(250);
            set_buzzer_tone(BUZZER_B, D4);
            sleep_ms(250);
        } else if (g > r && g > b && (g - r > dominance_threshold || g - b > dominance_threshold)) {
            printf("Cor detectada: Verde\n");
            strcpy(g_cor_detectada, "Verde"); // Atualiza cor
            set_buzzer_tone(BUZZER_B, E4);
            sleep_ms(250);
            set_buzzer_tone(BUZZER_B, G4);
            sleep_ms(250);
        } else if (b > r && b > g && (b - r > dominance_threshold || b - g > dominance_threshold)) {
            printf("Cor detectada: Azul\n");
            strcpy(g_cor_detectada, "Azul"); // Atualiza cor
            set_buzzer_tone(BUZZER_B, F4);
            sleep_ms(250);
            set_buzzer_tone(BUZZER_B, C4);
            sleep_ms(250);
        } else {
            printf("Cor desconhecida ou mista detectada: R:%d G:%d B:%d\n", r, g, b);
            strcpy(g_cor_detectada, "Desconhecida"); // Atualiza cor
            stop_buzzer(BUZZER_B);
        }
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000);
    npInit(LED_PIN);

    printf("Iniciando GY-33...\n");
    gy33_init(I2C_PORT, SDA_PIN, SCL_PIN);
    printf("GY-33 inicializado.\n");

    printf("Iniciando BH1750...\n");
    bh1750_power_on(I2C_PORT);
    printf("BH1750 inicializado.\n");

    init_buzzer_pwm(BUZZER_A);
    init_buzzer_pwm(BUZZER_B);
    init_rgb();

    // Configura o display
    setupDisplay();

     //inicia os botões
    buttons_init(buttonA);
    buttons_init(buttonB);

    // cria interrupção para botão A e B
    gpio_set_irq_enabled_with_callback(buttonB, GPIO_IRQ_EDGE_FALL, true, &debounce);
    gpio_set_irq_enabled(buttonA, GPIO_IRQ_EDGE_FALL, true);

    while (true) {
        uint8_t r, g, b;
        uint16_t c;
        
        // Corrigido para uint16_t para ler o valor completo de lux
        uint16_t luminosity = bh1750_read_measurement(I2C_PORT);
        printf("Lux = %d\n", luminosity);

        gy33_read_normalized_color(&r, &g, &b, &c);
        printf("RGB: (%d, %d, %d) | Clear: %d\n", r, g, b, c);

        // Processa os dados e atualiza as variáveis globais
        process_and_play_tone(r, g, b, c, luminosity);

        // Atualiza o display com os novos dados
        updateDisplay();

        set_rgb_led_digital(current_color_hex);
        update_luminosity_state(luminosity);
        npDisplayDigit(digit);
        
        sleep_ms(500);
    }
}