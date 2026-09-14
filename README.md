<div align="center">SPD CAN

Speeduino Telemetry & CAN Gateway

Gateway/Bridge de comunicação automotiva de alta performance

<p>
  <strong>Speeduino</strong>
  &nbsp;→&nbsp;
  <strong>UART / Bluetooth SPP</strong>
  &nbsp;→&nbsp;
  <strong>SPD CAN</strong>
  &nbsp;→&nbsp;
  <strong>CAN 1 Mbps</strong>
</p><p>
  <a href="#-funcionalidades">Funcionalidades</a> •
  <a href="#-arquitetura">Arquitetura</a> •
  <a href="#-hardware">Hardware</a> •
  <a href="#-protocolo-can">Protocolo CAN</a> •
  <a href="#-utilização">Utilização</a>
</p></div>---

📖 Sobre o Projeto

O SPD CAN é um firmware de gateway/bridge de comunicação automotiva de alta performance.

O projeto atua como um tradutor síncrono bidirecional, realizando requisições de telemetria para uma Unidade de Controle do Motor (ECU) de código aberto, Speeduino, através de:

- Interface Serial secundária (UART)
- Bluetooth Clássico (SPP)

Os dados recebidos são processados e convertidos para transmissão em tempo real em um barramento multiplexado CAN (Controller Area Network).

Além da transmissão de telemetria, o gateway também monitora o barramento CAN para receber comandos destinados ao acionamento direto de saídas digitais de alta velocidade, como:

- 2-Step
- Start / Partida

---

<div align="center">🔄 Comunicação Bidirecional

                    ┌───────────────────┐
                    │     SPEEDUINO     │
                    │        ECU        │
                    └─────────┬─────────┘
                              │
                   ┌──────────┴──────────┐
                   │                     │
                 UART              Bluetooth SPP
                   │                     │
                   └──────────┬──────────┘
                              │
                       ┌──────▼──────┐
                       │   SPD CAN   │
                       │   Gateway   │
                       └──────┬──────┘
                              │
                         CAN 1 Mbps
                              │
                     ┌────────▼────────┐
                     │  Barramento CAN │
                     └────────┬────────┘
                              │
                ┌─────────────┴─────────────┐
                │                           │
           Telemetria                  Comandos
                │                           │
        RPM / MAP / TPS                 2-Step
        ECT / BAT / etc.                Start
                                            │
                                    ┌───────▼───────┐
                                    │ Saídas Digitais│
                                    └───────────────┘

</div>---

🚀 Funcionalidades

<table>
<thead>
<tr>
<th>Função</th>
<th>Descrição</th>
</tr>
</thead>
<tbody>
<tr>
<td><strong>Comunicação Multimeio</strong></td>
<td>Suporte para conexão com a ECU através de UART física ou Bluetooth Clássico SPP.</td>
</tr>
<tr>
<td><strong>Varredura Ativa a 20 Hz</strong></td>
<td>Solicitação cíclica de dados em intervalos precisos de 50 ms.</td>
</tr>
<tr>
<td><strong>Processamento e Escalonamento</strong></td>
<td>Extração de variáveis binárias e conversão matemática para o formato utilizado pelo barramento CAN.</td>
</tr>
<tr>
<td><strong>Transmissão Multiframe</strong></td>
<td>Fragmentação automática de payloads superiores a 5 bytes em múltiplos quadros CAN.</td>
</tr>
<tr>
<td><strong>Controle via CAN</strong></td>
<td>Recepção de comandos CAN para acionamento das funções 2-Step e Start.</td>
</tr>
<tr>
<td><strong>Watchdog de Partida</strong></td>
<td>Desligamento automático da saída de partida após timeout de 100 ms sem sinal CAN válido.</td>
</tr>
<tr>
<td><strong>Diagnóstico</strong></td>
<td>Interface Serial USB a 115200 bps para monitoramento e depuração em tempo real.</td>
</tr>
</tbody>
</table>---

🧬 Arquitetura

O SPD CAN possui duas implementações principais:

<div align="center">Versão| Plataforma| ECU| CAN
ARDUINO-UART| Arduino AVR| UART 115200| MCP2515
ESP32-BLUETOOTH| ESP32 WROOM-32| Bluetooth SPP| MCP2515

</div>📁 Estrutura do Repositório

SPD-CAN/
│
├── README.md
├── LICENSE
│
├── ARDUINO-UART/
│   ├── ARDUINO-UART.ino
│   └── README.md
│
└── ESP32-BLUETOOTH/
    ├── ESP32-BLUETOOTH.ino
    └── README.md

---

🔌 ARDUINO-UART

<a href="./ARDUINO-UART"><strong>ARDUINO-UART</strong>

</a>Hardware

- Arduino Nano
- Arduino Uno
- Arduino Mega
- MCP2515 CAN

Comunicação

Speeduino
    │
    │ UART 115200
    ▼
Arduino
    │
    │ SPI
    ▼
MCP2515
    │
    │ CAN 1 Mbps
    ▼
Barramento CAN

Indicação

Versão indicada para montagens diretas através de cabo e utilização com microcontroladores Arduino AVR.

---

📡 ESP32-BLUETOOTH

<a href="./ESP32-BLUETOOTH"><strong>ESP32-BLUETOOTH</strong>

</a>Hardware

- ESP32 WROOM-32
- MCP2515 CAN

Comunicação

Speeduino
    │
    │ Bluetooth Classic / SPP
    ▼
ESP32
    │
    │ SPI
    ▼
MCP2515
    │
    │ CAN 1 Mbps
    ▼
Barramento CAN

Indicação

Versão destinada à utilização sem fio, eliminando o cabo físico entre a ECU e o gateway.

---

🛠️ Hardware

Barramento CAN

<table>
<thead>
<tr>
<th>Parâmetro</th>
<th>Configuração</th>
</tr>
</thead>
<tbody>
<tr>
<td>Velocidade</td>
<td><strong>1000 Kbps / 1 Mbps</strong></td>
</tr>
<tr>
<td>Controlador CAN</td>
<td><strong>MCP2515</strong></td>
</tr>
<tr>
<td>Frequência do cristal</td>
<td><strong>8 MHz</strong></td>
</tr>
<tr>
<td>Identificadores</td>
<td><strong>29 bits — Extended</strong></td>
</tr>
<tr>
<td>CS — Arduino</td>
<td><code>D10</code></td>
</tr>
<tr>
<td>CS — ESP32</td>
<td><code>GPIO 5</code></td>
</tr>
</tbody>
</table><details>
<summary><strong>⚠️ Atenção — cristal do MCP2515</strong></summary>Confirme a frequência do cristal instalado no módulo MCP2515 utilizado.

O firmware deve estar configurado de acordo com a frequência real do controlador CAN.

</details>---

📡 Comunicação com a ECU

<table>
<thead>
<tr>
<th>Parâmetro</th>
<th>Valor</th>
</tr>
</thead>
<tbody>
<tr>
<td>UART</td>
<td><strong>115200 bps</strong></td>
</tr>
<tr>
<td>Debug Serial</td>
<td><strong>115200 bps</strong></td>
</tr>
<tr>
<td>Bluetooth</td>
<td><strong>Bluetooth Classic SPP</strong></td>
</tr>
<tr>
<td>Modo Bluetooth</td>
<td><strong>Master</strong></td>
</tr>
<tr>
<td>Buffer de entrada</td>
<td><strong>150 bytes</strong></td>
</tr>
<tr>
<td>Frequência de requisição</td>
<td><strong>20 Hz</strong></td>
</tr>
<tr>
<td>Intervalo</td>
<td><strong>50 ms</strong></td>
</tr>
</tbody>
</table>---

📊 Protocolo CAN

A estrutura de empacotamento do SPD CAN utiliza como referência especificações técnicas de protocolos automotivos de domínio público.

Os identificadores das mensagens são construídos dinamicamente utilizando operações lógicas de deslocamento de bits (bitwise) a partir de constantes pré-definidas:

0x5020
0x02
0x02FF

---

📈 Telemetria

Transmissão FCAN

<table>
<thead>
<tr>
<th>Parâmetro</th>
<th>ID do Canal</th>
<th>Escalonamento</th>
<th>Unidade</th>
</tr>
</thead>
<tbody>
<tr>
<td><strong>Rotação do Motor</strong></td>
<td><code>0x0084</code></td>
<td>×1</td>
<td>rpm</td>
</tr>
<tr>
<td><strong>Ponto / Avanço de Ignição</strong></td>
<td><code>0x008E</code></td>
<td>×10</td>
<td>graus (°)</td>
</tr>
<tr>
<td><strong>Posição da Borboleta — TPS</strong></td>
<td><code>0x0002</code></td>
<td>×10</td>
<td>%</td>
</tr>
<tr>
<td><strong>Pressão Absoluta — MAP</strong></td>
<td><code>0x0004</code></td>
<td>×10</td>
<td>kPa</td>
</tr>
<tr>
<td><strong>Temperatura do Motor — ECT</strong></td>
<td><code>0x0008</code></td>
<td>×10</td>
<td>°C</td>
</tr>
<tr>
<td><strong>Tensão da Bateria — BAT</strong></td>
<td><code>0x0012</code></td>
<td>×10</td>
<td>V</td>
</tr>
<tr>
<td><strong>Duty Cycle de Injeção</strong></td>
<td><code>0x008A</code></td>
<td>×10</td>
<td>%</td>
</tr>
</tbody>
</table>---

🎛️ Comandos CAN

O gateway monitora continuamente o identificador:

<div align="center">"0x923013FF"

</div>Esse identificador é utilizado para comandos destinados às saídas físicas do hardware.

2-Step

A saída digital de 2-Step é acionada através da sequência:

FF 00 EC 00 01

Ao identificar o payload correspondente, o firmware realiza o acionamento da saída digital configurada.

---

Start / Partida

O comando de Start também é recebido através do barramento CAN.

Para garantir uma condição segura, o firmware possui um timeout de 100 ms.

Caso o sinal CAN deixe de ser recebido dentro da janela esperada, a saída de partida é automaticamente desligada.

CAN válido
    │
    ▼
START ON
    │
    │  Sem sinal > 100 ms
    ▼
START OFF

Esse mecanismo protege contra situações como:

- perda de comunicação;
- interrupção do dispositivo transmissor;
- falha de comunicação CAN;
- ausência de novos comandos.

---

🧩 Transmissão Multiframe

Payloads superiores a 5 bytes são automaticamente fragmentados em múltiplos quadros CAN.

Quadro de Abertura

O primeiro quadro utiliza o índice:

0x00

Ele informa o comprimento total do payload e transporta os primeiros 5 bytes úteis.

┌────────┬────────┬─────────────────────────┐
│ Índice │ Tamanho│      Dados — 5 bytes    │
├────────┼────────┼─────────────────────────┤
│  0x00  │  LEN   │ D0 D1 D2 D3 D4          │
└────────┴────────┴─────────────────────────┘

Quadros Consecutivos

Os quadros seguintes utilizam índices sequenciais:

0x01
0x02
0x03
...

Cada quadro pode transportar até 7 bytes de dados.

┌────────┬───────────────────────────────┐
│ Índice │      Dados — até 7 bytes      │
├────────┼───────────────────────────────┤
│  0x01  │ D5 D6 D7 D8 D9 D10 D11        │
└────────┴───────────────────────────────┘

O processo continua até que todo o payload tenha sido transmitido.

⏱️ Intervalo de Proteção

Entre os quadros é utilizado um intervalo de aproximadamente:

<div align="center">"200 µs"

</div>Esse intervalo proporciona uma margem de proteção entre as transmissões e contribui para manter o processamento previsível no receptor.

---

🔧 Utilização

1️⃣ Instalar a biblioteca

Instale a biblioteca:

MCP_CAN_lib

Repositório:

<a href="https://github.com/coryjfowler/MCP_CAN_lib">
https://github.com/coryjfowler/MCP_CAN_lib
</a>A biblioteca pode ser utilizada através da Arduino IDE ou PlatformIO.

---

2️⃣ Selecionar a versão

Escolha o diretório correspondente ao hardware:

/ARDUINO-UART

ou:

/ESP32-BLUETOOTH

---

3️⃣ Configurar os pinos

Verifique as definições no firmware:

#define CAN_CS

Também devem ser conferidos:

- pinos SPI;
- pinos das saídas digitais;
- CS do MCP2515;
- configuração da UART.

---

4️⃣ Configurar o Bluetooth

Na versão:

ESP32-BLUETOOTH

configure o nome ou endereço MAC do módulo Bluetooth da ECU Speeduino.

O ESP32 opera como:

Bluetooth Classic
SPP
Master

---

5️⃣ Gravar o firmware

Compile e faça o upload do firmware para o microcontrolador.

Após a gravação, abra o Monitor Serial em:

115200 bps

A interface de diagnóstico permite acompanhar a inicialização e o funcionamento do gateway.

---

🧪 Diagnóstico

A saída Serial USB pode ser utilizada para monitoramento durante desenvolvimento e testes.

Entre as informações que podem ser acompanhadas estão:

- Inicialização do MCP2515;
- Comunicação com a ECU;
- Comunicação UART;
- Conexão Bluetooth;
- Requisições de telemetria;
- Recebimento de dados;
- Processamento das variáveis;
- Transmissão CAN;
- Recepção de comandos;
- Acionamento das saídas;
- Eventos de timeout;
- Condições de erro.

---

⚡ Desempenho

<div align="center"><table>
<thead>
<tr>
<th>Parâmetro</th>
<th>Valor</th>
</tr>
</thead>
<tbody>
<tr>
<td>Frequência de requisição</td>
<td><strong>20 Hz</strong></td>
</tr>
<tr>
<td>Intervalo entre requisições</td>
<td><strong>50 ms</strong></td>
</tr>
<tr>
<td>Velocidade CAN</td>
<td><strong>1 Mbps</strong></td>
</tr>
<tr>
<td>Identificador CAN</td>
<td><strong>29 bits</strong></td>
</tr>
<tr>
<td>Intervalo Multiframe</td>
<td><strong>200 µs</strong></td>
</tr>
<tr>
<td>Timeout Start</td>
<td><strong>100 ms</strong></td>
</tr>
<tr>
<td>UART</td>
<td><strong>115200 bps</strong></td>
</tr>
<tr>
<td>Buffer</td>
<td><strong>150 bytes</strong></td>
</tr>
</tbody>
</table></div>---

⚠️ Segurança

O SPD CAN possui mecanismos destinados a reduzir riscos decorrentes de falhas de comunicação, incluindo timeout para o acionamento da saída de partida.

Mesmo assim, o firmware deve ser considerado um projeto de desenvolvimento, pesquisa e experimentação.

Antes da utilização em veículo, recomenda-se realizar testes controlados em bancada.

Devem ser verificadas, no mínimo:

- Alimentação;
- Aterramento;
- Níveis lógicos;
- Terminação CAN;
- Pinagem;
- Sentido das saídas;
- Comportamento durante perda de comunicação;
- Comportamento durante reset do microcontrolador;
- Comportamento durante falha da ECU;
- Comportamento durante falha do barramento CAN.

<details>
<summary><strong>⚠️ Atenção</strong></summary>Nunca utilize uma saída de acionamento de partida ou outra função crítica sem validar previamente seu comportamento em condições normais e de falha.

</details>---

⚖️ Aviso Legal / Disclaimer

Este é um projeto independente, de código aberto, desenvolvido com fins estritamente relacionados a estudo, pesquisa, desenvolvimento e experimentação de sistemas embarcados e comunicação automotiva.

Isenção de Vínculo

Este projeto foi desenvolvido de forma 100% autônoma e não possui qualquer tipo de afiliação, patrocínio, homologação ou vínculo comercial com fabricantes de módulos eletrônicos, marcas automotivas ou sistemistas.

Propriedade de Terceiros

Quaisquer mapeamentos de canais, constantes numéricas ou arquiteturas de rede implícitas baseiam-se em dados factuais de mercado e especificações públicas de engenharia, servindo exclusivamente para fins de interoperabilidade, estudo e desenvolvimento de sistemas de comunicação entre dispositivos distintos.

O uso de nomes, marcas, protocolos ou tecnologias de terceiros neste projeto não implica endosso, associação ou aprovação por seus respectivos proprietários.

Responsabilidade

O uso, aplicação, instalação ou modificação deste firmware em qualquer ambiente de testes ou automotivo é de total e exclusiva responsabilidade do usuário final.

O autor não se responsabiliza por:

- danos ao veículo;
- danos a módulos eletrônicos;
- danos ao motor;
- perda de dados;
- falhas de comunicação;
- funcionamento inadequado de equipamentos de terceiros;
- acidentes;
- prejuízos materiais ou pessoais decorrentes do uso do projeto.

O usuário é responsável por validar o hardware, firmware, conexões elétricas e comportamento do sistema antes de sua utilização.

---

📜 Licença

Consulte o arquivo ""LICENSE"" (./LICENSE) para obter as informações completas sobre os termos de utilização, modificação e distribuição do projeto.

---

<div align="center">👤 Autor

<strong>Sidney Himuro</strong>

<p>
Desenvolvido no âmbito da <strong>Himuro Performance</strong>.
</p><p>
Eletrônica Automotiva • Sistemas Embarcados • EFI • Telemetria • CAN
</p><br>⭐ <strong>SPD CAN — Speeduino Telemetry & CAN Gateway</strong>

</div>