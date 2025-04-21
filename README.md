# Sistema de Monitoramento de Temperatura e Incêndio para String Box
**Autor:** Daniel Silva de Souza

🎥 **Vídeo demonstrativo:** https://youtu.be/9FZJZ7f-6wM?si=kijlpPfXn_Z4Mnjq

Um sistema embarcado baseado no Raspberry Pi Pico para monitoramento em tempo real de temperatura e detecção de incêndio em sistemas fotovoltaicos, com exibição em display OLED, alerta sonoro via buzzer e indicação visual por LED RGB e matriz de LEDs.

---

## 📋 Características

- Display OLED SSD1306 (128x64)
- Leitura de temperatura via ADC (sensor analógico simulado)
- Detecção de incêndio simulada por botão
- Indicadores visuais:
  - LED RGB (normal, atenção e crítico)
  - Matriz de LEDs colorida
- Alerta sonoro com buzzer
- Joystick analógico com visualização da posição no display
- Relatório de evento crítico no terminal
- Interface interativa em tempo real via display

---

## 🔧 Hardware Necessário

- Raspberry Pi Pico
- Display OLED SSD1306 (I2C)
- Sensor de temperatura analógico (ou potenciômetro simulando)
- LED RGB (3 canais com controle PWM)
- 2 botões push-button
- Joystick analógico com botão central
- Matriz de LEDs WS2812
- Buzzer ativo
- Resistores pull-up para botões
- Cabos jumper e protoboard

---

## 📌 Pinagem

### 📟 I2C - Display OLED
- SDA: GPIO 14  
- SCL: GPIO 15

### 🕹️ Joystick
- Eixo X (ADC0): GPIO 26  
- Eixo Y (ADC1): GPIO 27  
- Botão (Push): GPIO 22

### 🔘 Botões
- Botão A: GPIO 5 (modo BOOTSEL)
- Botão B: GPIO 6 (simula incêndio)

### 🔴 LED RGB
- Vermelho: GPIO 11  
- Verde: GPIO 12  
- Azul: GPIO 13

### 🔊 Buzzer
- GPIO 21

### 🧱 Matriz de LEDs
- GPIO 7

---

## 🚀 Funcionalidades

- 📈 Leitura contínua da temperatura
- 🔥 Detecção de incêndio simulada via botão
- 🟢🟡🔴 Indicação por LED RGB:
  - Verde: temperatura normal
  - Amarelo: temperatura elevada
  - Vermelho: temperatura crítica / incêndio
- 🧠 Lógica de desligamento com contagem regressiva (visível na matriz)
- 📢 Alerta sonoro com buzzer (5 bipes)
- 🖥️ Exibição de status e joystick no terminal (via USB serial)
- 🧾 Geração automática de relatório ao detectar evento crítico

---

## 🧪 Funcionamento

- Temperatura < 40°C → Estado **NORMAL**
- Temperatura entre 40–59°C → Estado **ATENÇÃO**
- Temperatura ≥ 60°C ou fogo detectado → Estado **CRÍTICO**  
  → Aciona buzzer, mostra contagem na matriz e emite relatório

---

## 🛠️ Instalação

1. Clone o repositório:
   ```bash
   git clone https://github.com/seu-usuario/monitoramento-temperatura-incendio.git


   ```

2. Configure o ambiente de desenvolvimento Pico SDK

3. Compile o projeto:
   ```bash
   pico-sdk build
   ```


4. Carregue o arquivo .uf2 gerado no Raspberry Pi Pico

## 📦 Estrutura do Projeto

├── lib/
│   ├── ssd1306.h
│   ├── ssd1306.c
│   └── font.h
├── numeros.h          # Controle da matriz de LEDs (cores e números)
├── Main_Monitoramento_Temperatura_Incendio.c
├── CMakeLists.txt
└── README.md



## 📈 Melhorias Futuras

- [ ] Armazenamento em memória não-volátil
- [ ] Comunicação wireless
- [ ] Interface web
- [ ] Mais opções de sensores
- [ ] Exportação de dados
- [ ] Registro de histórico de eventos

## 📄 Licença

Este projeto está sob a licença MIT - veja o arquivo [LICENSE.md](LICENSE.md) para detalhes

## ✨ Contribuições

Contribuições são bem-vindas! Por favor, leia o [CONTRIBUTING.md](CONTRIBUTING.md) para detalhes sobre nosso código de conduta e o processo para enviar pull requests.

## 🤝 Suporte

Se você tiver alguma dúvida ou sugestão, por favor abra uma issue no repositório do projeto.
