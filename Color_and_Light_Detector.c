#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "pico/stdlib.h"
#include "buzzer.h"
#include "gy33.h"
#include "bh1750.h"

#define I2C_PORT i2c0
#define SDA_PIN 0
#define SCL_PIN 1



void detect_and_play_tone(uint8_t r, uint8_t g, uint8_t b, uint16_t clear_val, uint16_t light) {
    const int white_threshold = 100; 
    const int dominance_threshold = 30; 
    const int white_tolerance = 40; 

    if (light < 10) {
        printf("Ambiente muito escuro\n");
        set_buzzer_tone(BUZZER_A, C4); // Toca um tom baixo
        sleep_ms(250);
        set_buzzer_tone(BUZZER_A, D4); // Toca um tom baixo
        sleep_ms(250);
    } else{
        stop_buzzer(BUZZER_A); // Para o buzzer A
        if (clear_val > white_threshold && abs(r - g) < white_tolerance && abs(g - b) < white_tolerance) {
            printf("Cor detectada: Branco\n");
            set_buzzer_tone(BUZZER_B, D4); // Toca um tom baixo
            sleep_ms(250);
            set_buzzer_tone(BUZZER_B, E4); // Toca um tom baixo
            sleep_ms(250);
        } else if (r > g && r > b && (r - g > dominance_threshold || r - b > dominance_threshold)) {
            printf("Cor detectada: Vermelho\n");
            set_buzzer_tone(BUZZER_B, A4); // Toca um tom baixo
            sleep_ms(250);
            set_buzzer_tone(BUZZER_B, D4); // Toca um tom baixo
            sleep_ms(250);
        } else if (g > r && g > b && (g - r > dominance_threshold || g - b > dominance_threshold)) {
            printf("Cor detectada: Verde\n");
            set_buzzer_tone(BUZZER_B, E4); // Toca um tom baixo
            sleep_ms(250);
            set_buzzer_tone(BUZZER_B, G4); // Toca um tom baixo
            sleep_ms(250);
        } else if (b > r && b > g && (b - r > dominance_threshold || b - g > dominance_threshold)) {
            printf("Cor detectada: Azul\n");
            set_buzzer_tone(BUZZER_B, F4); // Toca um tom baixo
            sleep_ms(250);
            set_buzzer_tone(BUZZER_B, C4); // Toca um tom baixo
            sleep_ms(250);
        } else {
            printf("Cor desconhecida ou mista detectada: R:%d G:%d B:%d\n", r, g, b);
            stop_buzzer(BUZZER_B); // Para o buzzer B
        }
    }
}

int main() {
    stdio_init_all();
    sleep_ms(2000); // Um tempo para você abrir o monitor serial

    // Inicializa o sensor usando a nova função da biblioteca
    printf("Iniciando GY-33...\n");
    gy33_init(I2C_PORT, SDA_PIN, SCL_PIN);
    printf("GY-33 inicializado.\n");
  
    printf("Iniciando BH1750...\n");
    bh1750_power_on(I2C_PORT);
    printf("BH1750 inicializado.\n");

    // Inicializa seus buzzers
    init_buzzer_pwm(BUZZER_A);
    init_buzzer_pwm(BUZZER_B);

    while (true) {
        uint8_t r, g, b;
        uint16_t c;
        char luminosity = bh1750_read_measurement(I2C_PORT);;
        printf("Lux = %d\n", luminosity);

        // Lê a cor usando a função da biblioteca
        gy33_read_normalized_color(&r, &g, &b, &c);
        
        // Imprime os valores lidos
        printf("RGB: (%d, %d, %d) | Clear: %d\n", r, g, b, c);

        // Processa a cor lida
        detect_and_play_tone(r, g, b, c, luminosity);

        sleep_ms(500);
    }
}