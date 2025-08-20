# 🌈 Sistema de Detecção de Cor e Luminosidade com RP2040
<p align="center">Repositório dedicado a um sistema de detecção de cor e medição de intensidade luminosa utilizando a placa BitDogLab com RP2040. O sistema utiliza um sensor GY-33 para identificação de cores e um BH1750 para medir a luz ambiente, fornecendo feedback através de um display OLED, uma matriz de LEDs 5x5, um LED RGB e um buzzer.</p>

## 📋 Apresentação da tarefa
Para este trabalho, foi implementado um sistema embarcado capaz de analisar o ambiente em tempo real. O dispositivo captura dados de cor (RGB) e intensidade luminosa (lux) utilizando sensores dedicados via comunicação I2C. O sistema oferece uma interface de usuário clara através de um display OLED, controle por botões físicos, e múltiplos feedbacks para o usuário: visual (matriz de LEDs e LED RGB) e sonoro (buzzer), criando uma experiência interativa e informativa.

## 🎯 Objetivos

- Implementar a leitura de dados de cor (RGB) com o sensor GY-33.

- Integrar um sensor de luminosidade BH1750 para medir a luz ambiente em lux.

- Desenvolver uma interface de usuário em display OLED para exibir dados em tempo real.

- Criar um sistema de feedback visual em uma matriz de LEDs 5x5 para representar o nível de luminosidade.

- Implementar feedback sonoro com um buzzer para identificar diferentes cores e estados do sistema.

- Utilizar botões físicos para interação do usuário, como alteração de status e reset.

## 📚 Descrição do Projeto

Utilizou-se a placa BitDogLab com o microcontrolador RP2040 para criar um sistema de análise de ambiente. O sistema coleta dados de cor através do sensor GY-33 e dados de intensidade luminosa com o sensor BH1750, ambos conectados via I2C0. As informações processadas, como a cor predominante (Vermelho, Verde, Azul, Branco), os valores RGB brutos e a medição em lux, são exibidas em tempo real no display OLED SSD1306, conectado via I2C1.

O feedback ao usuário é multimodal. Uma matriz de LEDs 5x5, controlada via PIO (Programmable I/O), exibe uma "barra" vertical que cresce conforme a intensidade luminosa aumenta, oferecendo uma representação visual e intuitiva do nível de luz. Um buzzer emite padrões sonoros distintos para cada cor primária detectada, para a cor branca e para ambientes muito escuros, fornecendo confirmação auditiva.

O controle do sistema é feito por dois botões: o Botão A permite ao usuário alterar a cor de um LED RGB de status, enquanto o Botão B coloca o dispositivo em modo BOOTSEL para facilitar a reprogramação.

## 🚶 Integrantes do Projeto
- Bruna Alves de Barros
- Eder Renato da Silva Cardoso Casar
- Matheus Pereira Alves
- Mariana Farias Silva

## 📑 Funcionamento do Projeto
O sistema opera através de uma estrutura lógica organizada em um loop principal eficiente:

Inicialização: Configuração das duas interfaces I2C (uma para sensores, outra para o display), dos GPIOs para os botões e LEDs, das interrupções dos botões, do PIO para a matriz de LEDs e do buzzer.

Leitura de Sensores: No loop principal, o sistema coleta periodicamente os dados dos sensores GY-33 (valores R, G, B e Clear) e BH1750 (valor em lux).

Processamento de Dados: Uma função dedicada analisa os valores RGB para determinar a cor predominante com base em limiares de dominância. Outra função mapeia o valor de lux para um dos seis níveis de intensidade pré-definidos (de 0 a 5).

Interface de Usuário (OLED): O display é constantemente atualizado para mostrar o status do sistema ("Analisando", "Escuro"), a cor detectada, os valores RGB numéricos e a intensidade luminosa em lux.

Feedback Visual (Matriz de LED): Com base no nível de luminosidade calculado, o sistema acende um número correspondente de colunas na matriz 5x5, criando um gráfico de barras visual.

Feedback Sonoro (Buzzer): O sistema toca sequências de notas específicas para alertar sobre a cor detectada (vermelho, verde, azul, branco) ou se o ambiente está muito escuro para uma leitura precisa.

Controle por Botões: O sistema utiliza interrupções nos pinos GPIO para detectar pressionamentos de botão de forma eficiente, com um tratamento de debounce para evitar leituras múltiplas.

## 👁️ Observações
O sistema utiliza duas interfaces I2C separadas: I2C0 para os sensores GY-33 e BH1750, e I2C1 para o display OLED, evitando conflitos de endereço e otimizando a comunicação.

O controle da matriz de LEDs WS2812B é feito utilizando uma máquina de estados do PIO da RP2040.

Foi implementado um debounce de 200ms por software na rotina de interrupção dos botões para garantir uma resposta precisa e única a cada clique.

A lógica de detecção de cores compara a intensidade entre os canais R, G e B e utiliza um limiar para decidir se uma cor é dominante.

A detecção de "Branco" é feita verificando se a intensidade geral é alta e se os valores de R, G e B estão próximos uns dos outros.

▶️ Vídeo no youtube mostrando o funcionamento do programa na placa Raspberry Pi Pico
<p align="center">
<a href="https://youtube.com/shorts/QbM9pziTygg">Clique aqui para acessar o vídeo</a>
</p>
