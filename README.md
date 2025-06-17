
# Tarefa: Roteiro de FreeRTOS #1 - EmbarcaTech 2025

Autor: **João Fernandes**

Curso: Residência Tecnológica em Sistemas Embarcados

Instituição: EmbarcaTech - HBr

Campinas, Junho de 2025

---

Sistema Multitarefa com FreeRTOS na BitDogLabEste projeto demonstra a implementação de um sistema multitarefa embarcado utilizando o FreeRTOS em linguagem C na placa Raspberry Pi Pico (com BitDogLab). O sistema controla múltiplos periféricos de forma concorrente, gerenciando tarefas de LED, buzzer e botões de controle.

**Funcionalidades**
O sistema opera com as seguintes tarefas:
- LED RGB (GPIOs 15, 14, 13): Alterna ciclicamente entre as cores vermelho, verde e azul a cada 500ms.
- Buzzer (GPIO 12): Emite um bipe curto periodicamente a cada 1 segundo.
- Botão A (GPIO 10): Permite suspender ou retomar a tarefa do LED RGB.
- Botão B (GPIO 11): Permite suspender ou retomar a tarefa do buzzer.
- As tarefas são gerenciadas pelo FreeRTOS, utilizando vTaskCreate(), vTaskDelay(), vTaskSuspend() e vTaskResume() para controle de execução e agendamento.Hardware UtilizadoPlaca de Desenvolvimento: 
- Raspberry Pi Pico W ou Raspberry Pi Pico 2 (com placa de expansão BitDogLab).
- LED RGB: Conectado aos GPIOs 15 (vermelho), 14 (verde), 13 (azul).
- Buzzer: Conectado ao GPIO 12.
- Botão A (Controle do LED): Conectado ao GPIO 10.
- Botão B (Controle do Buzzer): Conectado ao GPIO 11.
- Cabo Micro-USB/USB-C: Para alimentação e gravação do firmware.

**Estrurura de organização**
  
├── CMakeLists.txt              # Script principal do CMake para build.
├── pico_sdk_import.cmake       # Arquivo para importar o pico-sdk.
├── FreeRTOS/                   # Repositório clonado do kernel FreeRTOS.
│   └── (conteúdo do FreeRTOS-Kernel)
├── src/
│   └── main.c                  # Código-fonte principal com as tarefas FreeRTOS.
├── include/
│   └── FreeRTOSConfig.h        # Arquivo de configuração do FreeRTOS.
└── build/                      # Diretório de build (será criado ao compilar).

**Como Usar e Compilar**
Siga estes passos para configurar, compilar e gravar o firmware na sua placa:
  - Crie a estrutura de diretórios conforme descrito acima. 
  - No terminal, na pasta onde você deseja criar o projeto:
    - mkdir meu_projeto_freertos
    - cd meu_projeto_freertos
    - mkdir src include build
    - git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git FreeRTOS
    - touch CMakeLists.txt pico_sdk_import.cmake src/main.c include/FreeRTOSConfig.h

**Conteúdo dos Arquivos:**
- Preencha src/main.c com o código fornecido anteriormente.
- Preencha include/FreeRTOSConfig.h com o código fornecido anteriormente (certifique-se de remover a linha #include "pico/stdlib.h" deste arquivo).
- Preencha CMakeLists.txt e pico_sdk_import.cmake com os códigos completos e corrigidos fornecidos anteriormente.
- Limpeza e Compilação:É crucial limpar o ambiente de build antes de compilar para evitar conflitos de cache. No terminal, a partir do diretório raiz do seu projeto (meu_projeto_freertos):
    - rm -rf build CMakeCache.txt CMakeFiles/ # Limpeza completa
    - mkdir build
    - cd build
    - cmake ..        # Configura o projeto CMake
    - make -j4        # Compila o projeto (usando 4 threads, ajuste conforme seu CPU)

**Gravar o Firmware (.uf2):**
- Após a compilação bem-sucedida, o arquivo .uf2 estará na pasta build/ (ex: RTOS_teste.uf2 ou meu_projeto_freertos.uf2).
- Entre em Modo Bootloader: Mantenha o botão BOOTSEL na sua BitDogLab pressionado enquanto a conecta ao seu computador via USB. Ela aparecerá como um dispositivo de armazenamento removível chamado RPI-RP2.
- Copie o Arquivo: Arraste e solte o arquivo .uf2 para o dispositivo RPI-RP2.
- Reiniciar: A placa irá se desconectar e reiniciar automaticamente, executando o seu novo firmware.Comportamento Esperado na PlacaApós a gravação, observe o seguinte:
  - O LED RGB começará a alternar entre vermelho, verde e azul.O buzzer emitirá bipes periodicamente.
  - Pressione o Botão A para pausar/retomar o ciclo do LED.
  - Pressione o Botão B para pausar/retomar os bipes do buzzer.

---

## 📜 Licença
GNU GPL-3.0.
