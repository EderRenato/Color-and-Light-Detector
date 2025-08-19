#include "gy33.h"

// --- Definições e Variáveis Privadas ---

#define GY33_I2C_ADDR   0x29

// Registradores do sensor
#define ENABLE_REG      0x80
#define ATIME_REG       0x81
#define CONTROL_REG     0x8F
#define CDATA_REG       0x94
#define RDATA_REG       0x96
#define GDATA_REG       0x98
#define BDATA_REG       0x9A

// Variável global estática para armazenar a porta I2C.
// "static" a torna visível apenas dentro deste arquivo.
static i2c_inst_t *i2c_instance;

// --- Funções Privadas ---

// Funções que só são usadas dentro desta biblioteca.
// Declaradas como "static" para não serem visíveis externamente.
static void gy33_write_register(uint8_t reg, uint8_t value) {
    uint8_t buffer[2] = {reg, value};
    i2c_write_blocking(i2c_instance, GY33_I2C_ADDR, buffer, 2, false);
}

static uint16_t gy33_read_register(uint8_t reg) {
    uint8_t buffer[2];
    i2c_write_blocking(i2c_instance, GY33_I2C_ADDR, &reg, 1, true);
    i2c_read_blocking(i2c_instance, GY33_I2C_ADDR, buffer, 2, false);
    return (buffer[1] << 8) | buffer[0];
}

// --- Implementação das Funções Públicas ---

void gy33_init(i2c_inst_t *i2c_port, uint8_t sda_pin, uint8_t scl_pin) {
    i2c_instance = i2c_port; // Armazena a instância I2C para uso futuro
    
    // Inicializa o hardware I2C
    i2c_init(i2c_instance, 100 * 1000);
    gpio_set_function(sda_pin, GPIO_FUNC_I2C);
    gpio_set_function(scl_pin, GPIO_FUNC_I2C);
    gpio_pull_up(sda_pin);
    gpio_pull_up(scl_pin);
    
    // Configura o sensor
    gy33_write_register(ENABLE_REG, 0x03);
    gy33_write_register(ATIME_REG, 0xF5);
    gy33_write_register(CONTROL_REG, 0x00);
}

void gy33_read_normalized_color(uint8_t *r, uint8_t *g, uint8_t *b, uint16_t *clear_val) {
    uint16_t red_raw, green_raw, blue_raw;
    
    *clear_val = gy33_read_register(CDATA_REG);
    red_raw = gy33_read_register(RDATA_REG);
    green_raw = gy33_read_register(GDATA_REG);
    blue_raw = gy33_read_register(BDATA_REG);

    if (*clear_val == 0) {
        *r = 0;
        *g = 0;
        *b = 0;
    } else {
        // Usa o valor do ponteiro clear_val para o cálculo
        *r = (uint8_t)((float)red_raw / *clear_val * 255.0);
        *g = (uint8_t)((float)green_raw / *clear_val * 255.0);
        *b = (uint8_t)((float)blue_raw / *clear_val * 255.0);
    }
}