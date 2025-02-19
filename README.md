
# 🕹️ Controle de LEDs e Display OLED com Joystick e Raspberry Pi Pico W

Este projeto demonstra como controlar LEDs (azul, vermelho e verde) e interagir com um display OLED usando um joystick e botões em uma Raspberry Pi Pico W. O joystick controla a intensidade dos LEDs e a posição de um retângulo no display. Os botões permitem alternar o estado do LED verde, mudar o estilo da borda do display e ativar/desativar o controle PWM dos LEDs.

---

## 🔧 **Hardware**

*   **Microcontrolador:** Raspberry Pi Pico W
*   **Componentes Principais:**
    *   Joystick (com botões)
    *   Display OLED (SSD1306)
    *   LEDs (azul, vermelho, verde)
*   **Conexões:**
    *   Cabo Micro-USB para alimentação e programação
    *   Jumpers para conectar os componentes à Pico W

### 📍 **Mapeamento de Pinos**

```
PINO_LED_AZUL: 12
PINO_LED_VERMELHO: 13
PINO_LED_VERDE: 11
PINO_JOYSTICK_Y: 26 (ADC0)
PINO_JOYSTICK_X: 27 (ADC1)
PINO_BOTAO_JOYSTICK: 22
PINO_BOTAO_A: 5
Display OLED SDA: 14
Display OLED SCL: 15
```

---

## 💻 **Software Necessário**

*   **SDK:** Raspberry Pi Pico SDK (v2.1.0 ou superior)
*   **Ferramentas:** CMake (v3.13+), VS Code (recomendado)
*   **Compilador:** GCC para ARM (fornecido pelo Pico SDK)
*   **Controle de Versão:** Git (opcional, mas recomendado)

---

## 📁 **Estrutura do Projeto**

```
.
├── .vscode/              # Configurações da IDE (VS Code)
├── build/                # Diretório de compilação (gerado pelo CMake)
├── Display_Bibliotecas/  # Bibliotecas para controlar o display OLED
│   ├── font.h
│   ├── ssd1306.c
│   └── ssd1306.h
├── generated/            # Diretório para arquivos gerados
├── .gitignore            # Arquivos ignorados pelo Git
├── CMakeLists.txt        # Configuração do CMake para o build
├── diagram.json          # Diagrama do circuito para simulação no Wokwi (opcional)
├── main.c                # Código fonte principal do projeto
├── pico_sdk_import.cmake # Importa o Pico SDK para o CMake
└── wokwi.toml            # Configuração para simulação no Wokwi (opcional)
```

---

## ⚡ **Como Executar**

### **Clone o Repositório**

```bash
git clone [URL do seu repositório]
cd [nome do diretório do seu repositório]
```

### **Compilação (Hardware Físico)**

```bash
mkdir build
cd build
cmake ..
make
```

### **Gravação no Pico W**

1.  Pressione o botão **BOOTSEL** na sua Pico W e conecte-a ao computador.
2.  A Pico W deve aparecer como um dispositivo de armazenamento USB.
3.  Copie o arquivo `joystick.uf2` (gerado na pasta `build/`) para a unidade USB da Pico W.

### **Simulação no Wokwi (Opcional)**

1.  Se você tiver um arquivo `diagram.json` e `wokwi.toml`, pode simular o projeto online em [Wokwi](https://wokwi.com/).
2.  Crie um novo projeto no Wokwi.
3.  Importe os arquivos `diagram.json` e `wokwi.toml`.
4.  Execute a simulação.

---

## 🚀 **Funcionalidades**

*   **Controle dos LEDs:**
    *   O eixo Y do joystick controla a intensidade do LED azul via PWM.
    *   O eixo X do joystick controla a intensidade do LED vermelho via PWM.
    *   O botão do joystick alterna o estado (ligado/desligado) do LED verde.
*   **Interação com o Display OLED:**
    *   O joystick controla a posição de um retângulo no display OLED.
    *   O botão do joystick alterna o estilo da borda do display (sem borda, borda simples, borda dupla, borda pontilhada).
    *   O botão A ativa/desativa o controle PWM dos LEDs (permitindo desligá-los completamente).
*   **Debounce nos Botões:** Implementação de debounce para evitar leituras múltiplas ao pressionar os botões.

---

## 📊 **Arquitetura do Código**

1.  **`main.c`:**
    *   **Inicialização:**
        *   Inicializa o I2C para comunicação com o display OLED.
        *   Inicializa o ADC para leitura dos eixos do joystick.
        *   Configura os pinos dos botões com interrupções (IRQs) para detectar os pressionamentos.
        *   Configura os pinos dos LEDs com PWM.
    *   **Loop Principal (`while(true)`):**
        *   Lê os valores dos eixos X e Y do joystick através do ADC.
        *   Calcula as diferenças em relação ao centro para determinar a intensidade dos LEDs e a posição do retângulo.
        *   Define o duty cycle do PWM para os LEDs azul e vermelho com base nos valores do joystick.
        *   Desenha o retângulo na nova posição no display OLED.
        *   Desenha a borda do display com o estilo selecionado.
        *   Envia os dados para atualizar o display.
    *   **Funções de Callback:**
        *   `callback_botao()`: Função chamada quando um dos botões é pressionado.  Verifica o tempo desde o último pressionamento para implementar o debounce.
    *   **Funções Auxiliares:**
        *   `inicializar_hardware()`:  Configura o I2C, ADC, e GPIOs dos botões.
        *   `processar_botao()`:  Realiza ações com base no botão pressionado (alternar o LED verde, mudar o estilo da borda, ativar/desativar o PWM).
        *   `desenhar_borda()`:  Desenha a borda do display OLED com o estilo selecionado.
        *   `inicializar_pwm()`:  Configura um pino GPIO para PWM e retorna a configuração.
        *   `definir_duty_pwm()`:  Define o duty cycle de um canal PWM.
        *   `calcular_intensidade()`:  Calcula a intensidade do LED com base na leitura do joystick, aplicando uma zona morta.

---

## 🐛 **Depuração**

*   **Conexões:** Verifique cuidadosamente todas as conexões entre a Raspberry Pi Pico W, o joystick, o display OLED e os LEDs.
*   **Serial:**  Adicione chamadas a `printf()` para imprimir valores de variáveis e ajudar a identificar problemas.  Lembre-se de que `stdio_usb` está habilitado, então a saída serial será enviada pela porta USB.
*   **Simulação:** Use o Wokwi para simular o circuito e o código. Isso pode ajudar a identificar problemas antes de gravar o código na Pico W.
*   **Valores do Joystick:**  Verifique os valores lidos do joystick pelo ADC.  Certifique-se de que o valor central (quando o joystick está em repouso) esteja próximo de `2048`.  Ajuste a `ZONA_MORTA` se necessário.
*   **Display OLED:** Verifique se o endereço I2C do display está correto (`0x3C`).

---

## 💡 Observações

*   O código utiliza PWM para controlar a intensidade dos LEDs, permitindo um efeito de variação suave.
*   A zona morta no joystick evita pequenas flutuações na leitura do ADC, garantindo um controle mais preciso.
*   O debounce dos botões impede que um único pressionamento seja interpretado como múltiplos eventos.
*   O projeto pode ser expandido para incluir mais funcionalidades, como menus no display OLED, controle de outros dispositivos ou comunicação sem fio.

---

## 🔗 **Diagrama do Circuito**

[Inserir aqui um diagrama do circuito.  Pode ser um link para uma imagem ou um diagrama criado com ferramentas como Fritzing ou similares.]

## 🔗 **Vídeo de Funcionamento**

[Inserir aqui um link para um vídeo demonstrando o projeto em funcionamento.]

## 📞 **Contato**

*   👤 **Autor:** [Seu Nome]
*   📧 **E-mail:** [Seu Email]

---
```

