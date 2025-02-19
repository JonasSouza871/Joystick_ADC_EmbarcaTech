#include "pico/stdlib.h"
#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/i2c.h"
#include "Display_Bibliotecas/ssd1306.h"
#include <stdlib.h>

// Configurações globais
#define ZONA_MORTA 200          // Zona morta aumentada para filtrar pequenos movimentos
#define TEMPO_DEBOUNCE 500      // Tempo mínimo entre eventos de botão (em ms)
#define LARGURA_DISPLAY 128     // Largura do display OLED
#define ALTURA_DISPLAY 64       // Altura do display OLED
#define VALOR_MAX_PWM 1250      // Valor máximo para o PWM (wrap)
#define CENTRO_JOYSTICK 2048    // Valor central do joystick (12 bits)
#define ADC_MAXIMO 4095         // Valor máximo da leitura do ADC (12 bits)

// Definição dos pinos
#define PINO_LED_AZUL 12
#define PINO_LED_VERMELHO 13
#define PINO_LED_VERDE 11
#define PINO_JOYSTICK_Y 26      // Pino ADC para o eixo Y do joystick
#define PINO_JOYSTICK_X 27      // Pino ADC para o eixo X do joystick
#define PINO_BOTAO_JOYSTICK 22  // Pino do botão do joystick
#define PINO_BOTAO_A 5          // Pino do botão A

// Estrutura para configuração do PWM
typedef struct {
    uint num_slice;   // Número do slice PWM
    uint canal;       // Canal PWM (0 ou 1)
} ConfigPWM;

// Variáveis globais
volatile bool led_verde_ligado = false; // Estado do LED verde
volatile uint8_t estilo_borda = 0;      // Estilo da borda (0-3)
volatile bool pwm_ativado = true;       // Controle do PWM
volatile uint32_t ultimo_tempo_botao = 0; // Último tempo de acionamento

// Estrutura para debounce de botões
typedef struct {
    uint pino;             // Pino GPIO do botão
    uint32_t ultimo_evento; // Tempo do último evento
} Botao;

Botao botoes[] = {{PINO_BOTAO_JOYSTICK, 0}, {PINO_BOTAO_A, 0}};

// Protótipos de funções
void inicializar_hardware();
void processar_botao(uint gpio);
void desenhar_borda(ssd1306_t *disp, uint8_t estilo);
ConfigPWM inicializar_pwm(uint pino);
void definir_duty_pwm(ConfigPWM cfg, uint16_t duty);
uint16_t calcular_intensidade(int16_t diferenca);
void callback_botao(uint gpio, uint32_t eventos);

int main() {
    stdio_init_all();
    inicializar_hardware();

    // Inicialização do display OLED
    i2c_inst_t *i2c = i2c1;
    ssd1306_t display;
    ssd1306_init(&display, LARGURA_DISPLAY, ALTURA_DISPLAY, false, 0x3C, i2c);
    ssd1306_config(&display);
    ssd1306_fill(&display, false);
    ssd1306_send_data(&display);

    // Configurações PWM para os LEDs
    ConfigPWM pwm_azul = inicializar_pwm(PINO_LED_AZUL);
    ConfigPWM pwm_vermelho = inicializar_pwm(PINO_LED_VERMELHO);
    ConfigPWM pwm_verde = inicializar_pwm(PINO_LED_VERDE);

    // Posição inicial do retângulo no display
    uint8_t novo_x = (LARGURA_DISPLAY / 2) - 4;
    uint8_t novo_y = (ALTURA_DISPLAY / 2) - 4;

    while(true) {
        // Leitura dos eixos do joystick
        adc_select_input(0);
        uint16_t leitura_y = adc_read();
        adc_select_input(1);
        uint16_t leitura_x = adc_read();

        // Cálculo das diferenças em relação ao centro
        int16_t diferenca_y = (int16_t)leitura_y - CENTRO_JOYSTICK;
        int16_t diferenca_x = (int16_t)leitura_x - CENTRO_JOYSTICK;

        // Controle de intensidade dos LEDs com PWM
        if(pwm_ativado) {
            definir_duty_pwm(pwm_azul, calcular_intensidade(diferenca_y));
            definir_duty_pwm(pwm_vermelho, calcular_intensidade(diferenca_x));
            definir_duty_pwm(pwm_verde, led_verde_ligado ? VALOR_MAX_PWM : 0);
            
            // Mapeamento das coordenadas do display com zona morta
            if(abs(diferenca_x) >= ZONA_MORTA)
                novo_x = ((uint32_t)leitura_x * (LARGURA_DISPLAY - 8)) / ADC_MAXIMO;
            if(abs(diferenca_y) >= ZONA_MORTA)
                novo_y = (ALTURA_DISPLAY - 8) - ((uint32_t)leitura_y * (ALTURA_DISPLAY - 8)) / ADC_MAXIMO;
        } else {
            // Desliga todos os LEDs se PWM desativado
            definir_duty_pwm(pwm_azul, 0);
            definir_duty_pwm(pwm_vermelho, 0);
            definir_duty_pwm(pwm_verde, 0);
        }

        // Atualização do display
        ssd1306_fill(&display, false);
        ssd1306_rect(&display, novo_y, novo_x, 8, 8, true, true);
        desenhar_borda(&display, estilo_borda);
        ssd1306_send_data(&display);

        sleep_ms(10);
    }
}

// Implementação das funções

void inicializar_hardware() {
    // Configuração do I2C para o display
    i2c_init(i2c1, 400000);
    gpio_set_function(14, GPIO_FUNC_I2C);
    gpio_set_function(15, GPIO_FUNC_I2C);
    gpio_pull_up(14);
    gpio_pull_up(15);

    // Inicialização do ADC para o joystick
    adc_init();
    adc_gpio_init(PINO_JOYSTICK_Y);
    adc_gpio_init(PINO_JOYSTICK_X);

    // Configuração dos botões com debounce
    for(int i = 0; i < sizeof(botoes)/sizeof(Botao); i++) {
        gpio_init(botoes[i].pino);
        gpio_set_dir(botoes[i].pino, GPIO_IN);
        gpio_pull_up(botoes[i].pino);
        gpio_set_irq_enabled_with_callback(botoes[i].pino, GPIO_IRQ_EDGE_FALL, true, &callback_botao);
    }
}

void callback_botao(uint gpio, uint32_t eventos) {
    uint32_t agora = to_ms_since_boot(get_absolute_time());
    for(int i = 0; i < sizeof(botoes)/sizeof(Botao); i++) {
        if(gpio == botoes[i].pino && (agora - botoes[i].ultimo_evento) > TEMPO_DEBOUNCE) {
            botoes[i].ultimo_evento = agora;
            processar_botao(gpio);
        }
    }
}

void processar_botao(uint gpio) {
    if(gpio == PINO_BOTAO_JOYSTICK) {
        led_verde_ligado = !led_verde_ligado;
        estilo_borda = (estilo_borda + 1) % 4; // Cicla entre 4 estilos de borda
    }
    else if(gpio == PINO_BOTAO_A) {
        pwm_ativado = !pwm_ativado; // Alterna estado do PWM
    }
}

void desenhar_borda(ssd1306_t *disp, uint8_t estilo) {
    switch(estilo) {
        case 1: // Borda sólida simples
            ssd1306_rect(disp, 0, 0, LARGURA_DISPLAY, ALTURA_DISPLAY, true, false);
            break;
            
        case 2: // Borda dupla
            ssd1306_rect(disp, 0, 0, LARGURA_DISPLAY, ALTURA_DISPLAY, true, false);
            ssd1306_rect(disp, 2, 2, LARGURA_DISPLAY-4, ALTURA_DISPLAY-4, true, false);
            break;
            
        case 3: // Borda pontilhada
            // Linha superior
            for(uint8_t x = 0; x < LARGURA_DISPLAY; x += 4) {
                ssd1306_pixel(disp, x, 0, true);
            }
            // Linha inferior
            for(uint8_t x = 0; x < LARGURA_DISPLAY; x += 4) {
                ssd1306_pixel(disp, x, ALTURA_DISPLAY-1, true);
            }
            // Linha esquerda
            for(uint8_t y = 0; y < ALTURA_DISPLAY; y += 4) {
                ssd1306_pixel(disp, 0, y, true);
            }
            // Linha direita
            for(uint8_t y = 0; y < ALTURA_DISPLAY; y += 4) {
                ssd1306_pixel(disp, LARGURA_DISPLAY-1, y, true);
            }
            break;
            
        default: // Sem borda
            break;
    }
}

ConfigPWM inicializar_pwm(uint pino) {
    ConfigPWM cfg;
    gpio_set_function(pino, GPIO_FUNC_PWM);
    cfg.num_slice = pwm_gpio_to_slice_num(pino);
    cfg.canal = pwm_gpio_to_channel(pino);
    pwm_set_clkdiv(cfg.num_slice, 4); // Divisor de clock para 125 MHz / 4 = 31.25 MHz
    pwm_set_wrap(cfg.num_slice, VALOR_MAX_PWM);
    pwm_set_enabled(cfg.num_slice, true);
    return cfg;
}

void definir_duty_pwm(ConfigPWM cfg, uint16_t duty) {
    pwm_set_chan_level(cfg.num_slice, cfg.canal, duty);
}

uint16_t calcular_intensidade(int16_t diferenca) {
    // Ignora pequenas variações dentro da zona morta
    if(abs(diferenca) < ZONA_MORTA) return 0;
    
    // Calcula a intensidade proporcional à variação além da zona morta
    int16_t valor_efetivo = abs(diferenca) - ZONA_MORTA;
    return (valor_efetivo * VALOR_MAX_PWM) / (ADC_MAXIMO/2 - ZONA_MORTA);
}