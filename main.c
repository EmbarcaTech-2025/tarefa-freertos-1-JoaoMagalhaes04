#include "FreeRTOS.h"   // Inclui a biblioteca principal do FreeRTOS, essencial para todas as funcionalidades do RTOS (Real-Time Operating System).
#include "task.h"       // Inclui as definições e funções para criação, controle e manipulação de tarefas (threads) do FreeRTOS.
#include "queue.h"      // Inclui o sistema de filas, usado para comunicação e sincronização entre tarefas. Embora não usado explicitamente neste código, é uma ferramenta comum no FreeRTOS.
#include "timers.h"     // Inclui as funcionalidades de software timers, que permitem agendar chamadas de função periodicamente ou após um atraso, sem bloquear as tarefas. Não usado diretamente aqui, mas muito útil.
#include <stdio.h>      // Inclui a biblioteca padrão de entrada/saída, usada principalmente para funções como 'printf' para depuração e feedback no console.
#include "pico/stdlib.h" // Inclui a biblioteca padrão do SDK do Raspberry Pi Pico, que fornece funções de inicialização e controle básicas para o hardware do Pico.
#include <stdint.h>     // Inclui definições de tipos de dados inteiros padronizados, como 'uint8_t', 'uint16_t', etc., garantindo tamanhos de variáveis consistentes entre diferentes plataformas.
#include <stdbool.h>    // Inclui a definição do tipo booleano 'bool' e seus valores 'true' e 'false'.
#include "hardware/gpio.h" // Inclui as funções específicas do SDK do Pico para controle de pinos GPIO (General Purpose Input/Output), como configurar direção e ler/escrever estados.
#include "hardware/pwm.h"  // Inclui as funções para controlar a modulação por largura de pulso (PWM), que é útil para gerar tons no buzzer ou controlar brilho de LEDs. Neste código, o buzzer é controlado via GPIO simples (ligar/desligar).

// --- Definições de Hardware ---
// Esta seção define as constantes para os números dos pinos GPIO da placa BitDogLab,
// tornando o código mais legível e fácil de modificar se os pinos mudarem.
#define LED_RGB_RED_PIN     11 // Pino GPIO conectado ao LED vermelho do RGB.
#define LED_RGB_GREEN_PIN   12 // Pino GPIO conectado ao LED verde do RGB.
#define LED_RGB_BLUE_PIN    13 // Pino GPIO conectado ao LED azul do RGB.
#define BUZZER_PIN          21 // Pino GPIO conectado ao buzzer.
#define BUTTON_A_PIN        5  // Pino GPIO conectado ao botão A.
#define BUTTON_B_PIN        6  // Pino GPIO conectado ao botão B.

// --- Handles de Tarefas ---
// Handles são ponteiros ou referências para as tarefas criadas. Eles são usados
// para controlar as tarefas, como suspendê-las, retomá-las, ou excluí-las.
TaskHandle_t xLedRgbTaskHandle = NULL; // Handle para a tarefa que controla o LED RGB. Inicializado como NULL.
TaskHandle_t xBuzzerTaskHandle = NULL; // Handle para a tarefa que controla o buzzer. Inicializado como NULL.

// --- Variáveis de Estado Globais (Controladas pelas Tarefas) ---
// Estas variáveis booleanas mantêm o estado atual das tarefas de LED e Buzzer.
// 'static' as torna visíveis apenas dentro deste arquivo e evita conflitos de nomes.
static bool led_task_suspended = false;   // True se a tarefa do LED RGB estiver suspensa, false caso contrário.
static bool buzzer_task_suspended = false; // True se a tarefa do buzzer estiver suspensa, false caso contrário.

// --- Funções Auxiliares de GPIO ---
// Estas são pequenas funções para configurar pinos GPIO, promovendo a reutilização de código
// e tornando as tarefas mais limpas ao abstrair detalhes de inicialização de hardware.

/**
 * @brief Configura um pino GPIO como saída.
 * @param gpio_pin O número do pino GPIO a ser configurado.
 */
static void gpio_configure_output(uint gpio_pin) {
    gpio_init(gpio_pin);       // Inicializa o pino GPIO especificado.
    gpio_set_dir(gpio_pin, GPIO_OUT); // Define a direção do pino como saída.
}

/**
 * @brief Configura um pino GPIO como entrada com pull-up.
 * Um resistor de pull-up interno é ativado para garantir que o pino tenha um estado
 * definido (HIGH) quando o botão não estiver pressionado (circuito aberto).
 * @param gpio_pin O número do pino GPIO a ser configurado.
 */
static void gpio_configure_input_pullup(uint gpio_pin) {
    gpio_init(gpio_pin);       // Inicializa o pino GPIO especificado.
    gpio_set_dir(gpio_pin, GPIO_IN); // Define a direção do pino como entrada.
    gpio_pull_up(gpio_pin);    // Ativa o resistor de pull-up interno para o pino.
}

// --- Tarefa de Controle do LED RGB ---
// Esta tarefa é responsável por alternar as cores do LED RGB (Vermelho, Verde, Azul) a cada segundo.
void led_rgb_task(void *pvParameters) {
    // Configura os pinos do LED RGB como saídas usando a função auxiliar.
    gpio_configure_output(LED_RGB_RED_PIN);
    gpio_configure_output(LED_RGB_GREEN_PIN);
    gpio_configure_output(LED_RGB_BLUE_PIN);

    uint8_t current_color_state = 0; // Variável para controlar o estado atual da cor (0: Vermelho, 1: Verde, 2: Azul).

    while (true) { // Loop infinito da tarefa (característico de tarefas FreeRTOS).
        // Desliga todos os LEDs antes de ligar a próxima cor.
        // Isso garante que apenas uma cor esteja acesa por vez e evita combinações indesejadas durante a transição.
        gpio_put(LED_RGB_RED_PIN, 0);
        gpio_put(LED_RGB_GREEN_PIN, 0);
        gpio_put(LED_RGB_BLUE_PIN, 0);

        switch (current_color_state) {
            case 0: // Estado Vermelho
                gpio_put(LED_RGB_RED_PIN, 1); // Acende o LED vermelho.
                break;
            case 1: // Estado Verde
                gpio_put(LED_RGB_GREEN_PIN, 1); // Acende o LED verde.
                break;
            case 2: // Estado Azul
                gpio_put(LED_RGB_BLUE_PIN, 1); // Acende o LED azul.
                break;
        }

        // Avança para o próximo estado de cor no ciclo (0 -> 1 -> 2 -> 0).
        current_color_state = (current_color_state + 1) % 3;
        
        // Coloca a tarefa para "dormir" por 1 segundo (1000 milissegundos).
        // `pdMS_TO_TICKS` converte milissegundos para ticks do FreeRTOS, garantindo portabilidade.
        vTaskDelay(pdMS_TO_TICKS(1000)); 
    }
}

// --- Tarefa de Controle do Buzzer ---
// Esta tarefa gera um bipe periódico no buzzer.
void buzzer_task(void *pvParameters) {
    gpio_configure_output(BUZZER_PIN); // Configura o pino do buzzer como saída.

    while (true) { // Loop infinito da tarefa.
        // Gera um bipe de 100ms.
        // O buzzer é ligado e desligado rapidamente para criar um som.
        // 50 ciclos de (1ms ON + 1ms OFF) = 50 * 2ms = 100ms.
        for (int i = 0; i < 50; i++) { 
            gpio_put(BUZZER_PIN, 1); // Liga o buzzer.
            vTaskDelay(pdMS_TO_TICKS(1)); // Mantém ligado por 1ms.
            gpio_put(BUZZER_PIN, 0); // Desliga o buzzer.
            vTaskDelay(pdMS_TO_TICKS(1)); // Mantém desligado por 1ms.
        }
        
        // Pausa de 2 segundos entre os bipes.
        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

// --- Tarefa de Monitoramento dos Botões ---
// Esta tarefa monitora o estado dos botões A e B e alterna a suspensão/retomada das tarefas de LED e Buzzer.
void button_monitor_task(void *pvParameters) {
    // Configura os pinos dos botões como entrada com pull-up.
    gpio_configure_input_pullup(BUTTON_A_PIN);
    gpio_configure_input_pullup(BUTTON_B_PIN);

    // Variáveis para implementar a lógica de debounce (anti-repiques).
    // Elas armazenam o estado anterior dos botões para detectar quando um botão é pressionado (borda de subida).
    bool button_a_last_state = false;
    bool button_b_last_state = false;

    while (true) { // Loop infinito da tarefa.
        // Lê o estado atual dos botões.
        // O '!' (NOT lógico) é usado porque os botões são configurados com pull-up,
        // o que significa que o pino está HIGH (1) quando o botão está solto e LOW (0) quando pressionado.
        bool button_a_current_state = !gpio_get(BUTTON_A_PIN);
        bool button_b_current_state = !gpio_get(BUTTON_B_PIN);

        // --- Lógica para o Botão A (Controla o LED RGB) ---
        // Detecta uma "borda de subida": o botão está atualmente pressionado E estava solto antes.
        if (button_a_current_state && !button_a_last_state) {
            // Verifica se o handle da tarefa do LED é válido antes de tentar controlá-la.
            // Isso previne erros se a tarefa não foi criada com sucesso.
            if (xLedRgbTaskHandle != NULL) { 
                if (led_task_suspended) { // Se a tarefa do LED estiver suspensa...
                    vTaskResume(xLedRgbTaskHandle); // ...retoma a tarefa.
                    led_task_suspended = false;      // Atualiza o estado da variável.
                    printf("LED RGB: Retomado\n");  // Imprime mensagem no console serial.
                } else { // Se a tarefa do LED estiver ativa...
                    vTaskSuspend(xLedRgbTaskHandle); // ...suspende a tarefa.
                    led_task_suspended = true;       // Atualiza o estado da variável.
                    // Desliga os LEDs imediatamente quando a tarefa é suspensa,
                    // garantindo que eles não permaneçam em um estado ligado.
                    gpio_put(LED_RGB_RED_PIN, 0);
                    gpio_put(LED_RGB_GREEN_PIN, 0);
                    gpio_put(LED_RGB_BLUE_PIN, 0);
                    printf("LED RGB: Suspenso\n"); // Imprime mensagem no console serial.
                }
            }
        }

        // --- Lógica para o Botão B (Controla o Buzzer) ---
        // Detecta uma "borda de subida" para o botão B.
        if (button_b_current_state && !button_b_last_state) {
            // Verifica se o handle da tarefa do buzzer é válido.
            if (xBuzzerTaskHandle != NULL) { 
                if (buzzer_task_suspended) { // Se a tarefa do buzzer estiver suspensa...
                    vTaskResume(xBuzzerTaskHandle);  // ...retoma a tarefa.
                    buzzer_task_suspended = false;    // Atualiza o estado.
                    printf("Buzzer: Retomado\n");   // Imprime mensagem.
                } else { // Se a tarefa do buzzer estiver ativa...
                    vTaskSuspend(xBuzzerTaskHandle); // ...suspende a tarefa.
                    buzzer_task_suspended = true;     // Atualiza o estado.
                    gpio_put(BUZZER_PIN, 0);         // Desliga o buzzer imediatamente.
                    printf("Buzzer: Suspenso\n");   // Imprime mensagem.
                }
            }
        }

        // Atualiza os estados anteriores dos botões para a próxima iteração do loop.
        button_a_last_state = button_a_current_state;
        button_b_last_state = button_b_current_state;

        // Pequeno atraso para implementar o debounce. Isso evita que múltiplos eventos
        // de "pressionar botão" sejam registrados devido ao ruído elétrico quando o botão é pressionado/solto.
        vTaskDelay(pdMS_TO_TICKS(50)); 
    }
}

// --- Tarefa de Status do Sistema ---
// Esta tarefa imprime periodicamente o estado atual das outras tarefas no console serial.
void system_status_task(void *pvParameters) {
    while (true) { // Loop infinito da tarefa.
        printf("\n--- Status do Sistema ---\n"); // Cabeçalho do status.
        // Usa o operador ternário para imprimir "SUSPENSO" ou "ATIVO" com base nas variáveis de estado.
        printf("LED RGB: %s\n", led_task_suspended ? "SUSPENSO" : "ATIVO"); 
        printf("Buzzer:  %s\n", buzzer_task_suspended ? "SUSPENSO" : "ATIVO");
        printf("Controles:\n");
        printf("  Botão A: Alterna LED RGB\n");
        printf("  Botão B: Alterna Buzzer\n");
        printf("-------------------------\n"); // Rodapé do status.

        vTaskDelay(pdMS_TO_TICKS(5000)); // Espera 5 segundos antes de imprimir o status novamente.
    }
}

// --- Função Principal ---
// A função `main` é o ponto de entrada do programa. Ela inicializa o hardware,
// cria as tarefas do FreeRTOS e inicia o escalonador.
int main() {
    stdio_init_all(); // Inicializa todas as interfaces de I/O padrão do Pico, incluindo a UART para comunicação serial (o 'printf').

    printf("\n=== Sistema Multi-Tarefa BitDogLab ===\n"); // Mensagem de boas-vindas.
    printf("Inicializando FreeRTOS...\n"); // Mensagem de inicialização.

    // Criação das Tarefas
    // As tarefas são criadas com diferentes prioridades:
    // - `button_monitor_task` tem a prioridade mais alta (3) para garantir resposta rápida aos botões.
    // - `led_rgb_task` e `buzzer_task` têm prioridade média (2).
    // - `system_status_task` tem a prioridade mais baixa (1), pois é menos crítica.
    BaseType_t xReturned; // Variável para armazenar o status de retorno da criação da tarefa.

    // Cria a tarefa `led_rgb_task`.
    // Parâmetros: (função da tarefa, nome, tamanho da stack, parâmetros, prioridade, handle).
    xReturned = xTaskCreate(led_rgb_task, "LED_RGB", 256, NULL, 2, &xLedRgbTaskHandle);
    if (xReturned != pdPASS) { // Verifica se a criação da tarefa foi bem-sucedida.
        printf("ERRO: Falha ao criar a tarefa LED_RGB\n");
        for(;;); // Loop infinito de segurança: o sistema trava se uma tarefa crítica não puder ser criada.
    }

    // Cria a tarefa `buzzer_task`.
    xReturned = xTaskCreate(buzzer_task, "Buzzer", 256, NULL, 2, &xBuzzerTaskHandle);
    if (xReturned != pdPASS) {
        printf("ERRO: Falha ao criar a tarefa Buzzer\n");
        for(;;);
    }

    // Cria a tarefa `button_monitor_task`.
    xReturned = xTaskCreate(button_monitor_task, "Button_Monitor", 256, NULL, 3, NULL); // Não precisamos de handle para esta tarefa, pois ela não será suspensa/retomada externamente.
    if (xReturned != pdPASS) {
        printf("ERRO: Falha ao criar a tarefa Button_Monitor\n");
        for(;;);
    }

    // Cria a tarefa `system_status_task`.
    xReturned = xTaskCreate(system_status_task, "System_Status", 512, NULL, 1, NULL); // Stack um pouco maior devido ao uso de `printf` repetidamente.
    if (xReturned != pdPASS) {
        printf("ERRO: Falha ao criar a tarefa System_Status\n");
        for(;;);
    }

    printf("Todas as tarefas foram criadas com sucesso!\n");
    printf("Pressione os botões para controlar o sistema.\n");

    vTaskStartScheduler(); // Inicia o escalonador do FreeRTOS.
                           // A partir deste ponto, o controle do programa passa para o FreeRTOS,
                           // que gerencia a execução das tarefas com base em suas prioridades e estado.

    // Este ponto do código *nunca* deve ser alcançado em um sistema FreeRTOS em execução normal.
    // Se for alcançado, significa que o escalonador parou inesperadamente, indicando um erro grave.
    printf("ERRO FATAL: O escalonador FreeRTOS parou inesperadamente!\n");
    for(;;); // Loop infinito para travar o programa em caso de falha crítica.
}
