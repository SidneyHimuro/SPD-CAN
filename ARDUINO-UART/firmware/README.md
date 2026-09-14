<div align="center">SPD CAN — Arduino UART

Gateway SPD CAN para Arduino + Secondary Serial

<strong>Speeduino → UART → Arduino → MCP2515 → CAN</strong>

<p>
  <a href="#-visão-geral">Visão Geral</a> •
  <a href="#-hardware-e-pinagem">Hardware</a> •
  <a href="#-comunicação-uart">UART</a> •
  <a href="#-protocolo-can">CAN</a> •
  <a href="#-compilação">Compilação</a>
</p></div>---

📌 Visão Geral

Esta pasta contém a implementação ARDUINO-UART do gateway SPD CAN.

O firmware utiliza um Arduino equipado com MCP2515 para realizar a comunicação entre a ECU Speeduino, através da Secondary Serial (UART), e um barramento CAN de 1 Mbps.

O Arduino realiza requisições periódicas de telemetria para a Speeduino, recebe o pacote de dados, extrai as principais variáveis do motor, realiza os respectivos cálculos e escalonamentos e transmite as informações através do protocolo CAN utilizado pelo SPD CAN.

Além da transmissão de telemetria, o Arduino monitora o barramento CAN para receber comandos destinados às saídas digitais de:

- 2-Step
- Start / Partida

---

🔄 Arquitetura

<div align="center">┌──────────────────────┐
│      SPEEDUINO       │
│         ECU          │
└──────────┬───────────┘
           │
           │ Secondary Serial
           │ UART 115200
           ▼
┌──────────────────────┐
│       ARDUINO        │
│      SPD CAN         │
│                      │
│ • Requisição 20 Hz   │
│ • Processamento      │
│ • Escalonamento      │
│ • Controle de saída  │
└──────────┬───────────┘
           │
           │ SPI
           ▼
┌──────────────────────┐
│       MCP2515        │
│    CAN Controller    │
└──────────┬───────────┘
           │
           │ CAN 1 Mbps
           ▼
┌──────────────────────┐
│    Barramento CAN    │
└──────────┬───────────┘
           │
      ┌────┴─────┐
      │          │
 Telemetria   Comandos
      │          │
      │       ┌──┴───┐
      │       │      │
      ▼       ▼      ▼
     CAN    2-Step  Start

</div>---

🛠️ Hardware e Pinagem

O firmware foi desenvolvido para utilização com Arduino compatível com "Serial1", como o Arduino Mega, além de um controlador CAN externo MCP2515.

🔌 Pinagem

<table>
<thead>
<tr>
<th>Função</th>
<th>Pino</th>
<th>Descrição</th>
</tr>
</thead><tbody><tr>
<td><strong>MCP2515 CS</strong></td>
<td><code>D10</code></td>
<td>Chip Select da interface SPI</td>
</tr><tr>
<td><strong>2-Step OUT</strong></td>
<td><code>D13</code></td>
<td>Saída digital para acionamento do 2-Step</td>
</tr><tr>
<td><strong>Start OUT</strong></td>
<td><code>D4</code></td>
<td>Saída digital para acionamento da partida</td>
</tr><tr>
<td><strong>Serial Debug</strong></td>
<td><code>Serial</code></td>
<td>USB / UART principal — 115200 bps</td>
</tr><tr>
<td><strong>Speeduino UART</strong></td>
<td><code>Serial1</code></td>
<td>Secondary Serial — 115200 bps</td>
</tr></tbody>
</table>---

⚠️ Sobre o "Serial1"

O código utiliza:

Serial1.begin(115200);

Portanto, a placa precisa possuir uma interface Serial1 disponível.

No Arduino Mega, por exemplo:

Serial  → USB / Debug
Serial1 → Speeduino

Isso permite manter a comunicação com a Speeduino separada do terminal de diagnóstico.

<details>
<summary><strong>Arduino Uno / Nano</strong></summary>O Arduino Uno e o Nano AVR tradicionais possuem apenas uma UART física ("Serial").

Por isso, o código atual, que utiliza diretamente:

Serial1

é destinado a placas que possuam uma segunda UART de hardware.

Para utilizar esta implementação em um Uno ou Nano AVR, o firmware precisaria ser adaptado para outra interface serial.

</details>---

📡 Comunicação com a Speeduino

A comunicação com a ECU ocorre através da Secondary Serial.

<table>
<thead>
<tr>
<th>Parâmetro</th>
<th>Configuração</th>
</tr>
</thead><tbody><tr>
<td>Interface</td>
<td><strong>Serial1</strong></td>
</tr><tr>
<td>Baud Rate</td>
<td><strong>115200 bps</strong></td>
</tr><tr>
<td>Comando de requisição</td>
<td><code>'n'</code></td>
</tr><tr>
<td>Frequência de requisição</td>
<td><strong>20 Hz</strong></td>
</tr><tr>
<td>Intervalo</td>
<td><strong>50 ms</strong></td>
</tr><tr>
<td>Buffer</td>
<td><strong>150 bytes</strong></td>
</tr></tbody>
</table>---

🔄 Ciclo de Requisição

A cada 50 ms, o firmware executa uma nova requisição de dados.

O comando enviado para a Speeduino é:

const char REQUEST_COMMAND = 'n';

Portanto:

Arduino
   │
   │ 'n'
   ▼
Speeduino
   │
   │ Pacote de dados
   ▼
Arduino

Antes de enviar uma nova requisição, o firmware limpa os dados que eventualmente permaneçam no buffer:

while (Serial1.available())
    Serial1.read();

Em seguida:

Serial1.write(REQUEST_COMMAND);

O contador de bytes recebidos é reiniciado:

bytesReceived = 0;

---

📦 Buffer de Recepção

O firmware utiliza um buffer de:

const int BUFFER_SIZE = 150;

Portanto:

150 bytes

podem ser armazenados no buffer de entrada.

Caso sejam recebidos mais bytes que a capacidade disponível, os bytes excedentes são descartados para evitar escrita fora dos limites do buffer.

---

🧮 Processamento da Speeduino

O firmware somente processa o pacote quando:

bytesReceived >= 80

A função responsável pelo processamento é:

processSpeeduinoData();

As informações são extraídas diretamente de posições específicas do pacote recebido.

---

📊 Variáveis Extraídas

RPM

A rotação é formada pelos bytes:

dataBuffer[14]
dataBuffer[15]

Código:

uint16_t rpm =
    (dataBuffer[15] << 8) |
    dataBuffer[14];

O valor é utilizado diretamente:

RPM FCAN = RPM

---

🔥 Avanço de Ignição

O avanço é obtido diretamente de:

dataBuffer[8]

Código:

uint8_t advance = dataBuffer[8];

Antes da transmissão CAN:

Advance FCAN = Advance × 10

---

🌡️ MAP

O valor do MAP é formado por:

dataBuffer[4]
dataBuffer[5]

Código:

uint16_t map =
    (dataBuffer[5] << 8) |
    dataBuffer[4];

Escalonamento:

MAP FCAN = MAP × 10

---

🦋 TPS

O TPS é calculado através do byte:

dataBuffer[25]

Fórmula utilizada:

float tps =
    (dataBuffer[25] * 100L) / 200.0;

O resultado é então convertido para o formato FCAN:

TPS FCAN = TPS × 10

---

🌡️ ECT

A temperatura do líquido de arrefecimento é obtida através de:

dataBuffer[7]

Fórmula:

int ect =
    (int)dataBuffer[7] - 40;

Portanto:

ECT = dataBuffer[7] − 40

Escalonamento CAN:

ECT FCAN = ECT × 10

---

💉 Pulse Width / Duty Cycle

O Pulse Width é formado através dos bytes:

dataBuffer[76]
dataBuffer[77]

Código:

uint16_t pwRaw =
    (dataBuffer[77] << 8) |
    dataBuffer[76];

Conversão:

float pw = pwRaw / 1000.0;

O Duty Cycle é calculado a partir do Pulse Width e RPM:

if (rpm > 0)
{
    dutyCycle = (pw * rpm) / 600.0;
}

Finalmente:

PW FCAN = Duty Cycle × 10

<details>
<summary><strong>Fórmula utilizada</strong></summary>Duty Cycle = (PW × RPM) / 600

Quando "RPM = 0", o Duty Cycle é definido como:

0

</details>---

🔋 Tensão da Bateria

A tensão da bateria é obtida através de:

dataBuffer[9]

Código:

int batt = dataBuffer[9];

Escalonamento:

BAT FCAN = BAT × 10

---

📡 Protocolo CAN

O MCP2515 é inicializado com:

CAN.begin(
    MCP_ANY,
    CAN_1000KBPS,
    MCP_8MHZ
);

Portanto, a configuração utilizada é:

<table>
<thead>
<tr>
<th>Parâmetro</th>
<th>Valor</th>
</tr>
</thead><tbody><tr>
<td>Controlador</td>
<td><strong>MCP2515</strong></td>
</tr><tr>
<td>Velocidade CAN</td>
<td><strong>1 Mbps</strong></td>
</tr><tr>
<td>Cristal</td>
<td><strong>8 MHz</strong></td>
</tr><tr>
<td>Modo</td>
<td><strong>Normal</strong></td>
</tr><tr>
<td>Identificador</td>
<td><strong>Extended — 29 bits</strong></td>
</tr></tbody>
</table>Após a inicialização:

CAN.setMode(MCP_NORMAL);

---

🆔 Construção do CAN ID

O identificador utilizado para transmissão é construído a partir de três campos:

const uint16_t PRODUCT_ID = 0x5020;
const uint8_t DATAFIELD_ID = 0x02;
const uint16_t MESSAGE_ID = 0x02FF;

O ID final é calculado através de:

canID =
    ((uint32_t)PRODUCT_ID << 14) |
    ((uint32_t)DATAFIELD_ID << 11) |
    MESSAGE_ID;

Estrutura conceitual:

┌──────────────────┬──────────────┬───────────────┐
│    PRODUCT_ID    │ DATAFIELD_ID │  MESSAGE_ID   │
│     0x5020       │     0x02     │    0x02FF     │
└──────────────────┴──────────────┴───────────────┘

---

📊 Mapeamento de Telemetria

As medições são adicionadas ao payload através da função:

addMeasure()

Cada medida utiliza:

2 bytes → Measure ID
2 bytes → Value

Total:

4 bytes por medida

O payload atual contém 7 medidas:

<table>
<thead>
<tr>
<th>Parâmetro</th>
<th>Measure ID</th>
<th>Escalonamento</th>
</tr>
</thead><tbody><tr>
<td><strong>RPM</strong></td>
<td><code>0x0084</code></td>
<td>×1</td>
</tr><tr>
<td><strong>Avanço</strong></td>
<td><code>0x008E</code></td>
<td>×10</td>
</tr><tr>
<td><strong>TPS</strong></td>
<td><code>0x0002</code></td>
<td>×10</td>
</tr><tr>
<td><strong>MAP</strong></td>
<td><code>0x0004</code></td>
<td>×10</td>
</tr><tr>
<td><strong>ECT</strong></td>
<td><code>0x0008</code></td>
<td>×10</td>
</tr><tr>
<td><strong>Battery</strong></td>
<td><code>0x0012</code></td>
<td>×10</td>
</tr><tr>
<td><strong>Injection Duty</strong></td>
<td><code>0x008A</code></td>
<td>×10</td>
</tr></tbody>
</table>---

🧬 Estrutura do Payload

Como cada medida utiliza 4 bytes:

7 medidas × 4 bytes = 28 bytes

O payload transmitido possui atualmente:

28 bytes

Estrutura:

┌──────────────┬──────────────┐
│ Measure ID   │ Value        │
│ 2 bytes      │ 2 bytes      │
└──────────────┴──────────────┘
          × 7 medidas

Exemplo conceitual:

[ID RPM] [RPM]
[ID ADV] [ADV]
[ID TPS] [TPS]
[ID MAP] [MAP]
[ID ECT] [ECT]
[ID BAT] [BAT]
[ID DUTY][DUTY]

---

🧩 Transmissão Multiframe

O payload de 28 bytes não cabe em um único quadro CAN de 8 bytes.

Por isso, a função:

sendFCAN()

realiza automaticamente a fragmentação.

Quadro de Abertura

O primeiro quadro contém:

Byte 0 → índice 0x00
Byte 1 → tamanho
Byte 2 → tamanho
Byte 3-7 → primeiros 5 bytes

Estrutura:

┌───────┬─────────┬─────────────────────┐
│ 0x00  │   LEN   │ D0 D1 D2 D3 D4      │
└───────┴─────────┴─────────────────────┘

---

Quadros Consecutivos

Os quadros seguintes utilizam:

0x01
0x02
0x03
...

Cada um transporta até 7 bytes:

┌───────┬───────────────────────────────┐
│ INDEX │ DADOS — até 7 bytes           │
└───────┴───────────────────────────────┘

Entre os quadros é aplicado:

delayMicroseconds(200);

Portanto:

Intervalo de proteção: "200 µs"

---

🎛️ Comandos Recebidos via CAN

O firmware monitora continuamente o identificador:

0x923013FF

Os comandos são processados pela função:

readButtonCAN();

---

🟢 2-Step

O comando de 2-Step utiliza o payload:

FF 00 EC 00 XX

onde:

XX = 01 → ON
XX = 00 → OFF

A saída correspondente é:

Arduino D13

Código lógico:

twoStepButton = (rxBuf[4] == 0x01);

E:

digitalWrite(
    PIN_2STEP_OUT,
    twoStepButton ? HIGH : LOW
);

---

🏁 Start / Partida

O comando de partida utiliza:

FF 00 FE 00 XX

onde:

XX = 01 → START ON
XX = 00 → START OFF

A saída é:

Arduino D4

---

⏱️ Segurança da Partida

Quando o comando de Start é recebido com:

XX = 01

o firmware atualiza:

lastStartMessage = millis();

e liga a saída:

digitalWrite(PIN_START_OUT, HIGH);

O sistema possui um timeout de:

const unsigned long START_TIMEOUT = 100;

ou:

<div align="center">"100 ms"

</div>Se nenhum novo comando válido de Start for recebido nesse período:

digitalWrite(PIN_START_OUT, LOW);

Portanto:

CAN START
    │
    ▼
START ON
    │
    │
    │ < 100 ms
    │
    ▼
Novo comando?
   │
 ┌─┴─┐
SIM  NÃO
 │    │
 ▼    ▼
ON   OFF

Esse mecanismo impede que a saída permaneça acionada indefinidamente caso a comunicação CAN seja interrompida.

---

🖥️ Interface de Diagnóstico

A interface "Serial" principal é utilizada para diagnóstico:

Serial.begin(115200);

Durante a inicialização do CAN, enquanto o MCP2515 não estiver disponível, o firmware informa:

CAN FAIL

Após a inicialização bem-sucedida:

FCAN ONLINE

O Monitor Serial deve ser configurado para:

115200 bps

---

🔧 Compilação

Dependências

O projeto utiliza:

SPI

#include <SPI.h>

Biblioteca padrão utilizada pela plataforma Arduino.

MCP2515

#include <mcp_can.h>

Biblioteca:

<strong>MCP_CAN_lib</strong>

Repositório:

<a href="https://github.com/coryjfowler/MCP_CAN_lib">
https://github.com/coryjfowler/MCP_CAN_lib
</a>---

🚀 Como Compilar e Gravar

1. Instale a Arduino IDE

Utilize uma versão compatível com a placa Arduino escolhida.

---

2. Instale a biblioteca MCP_CAN_lib

Adicione a biblioteca MCP_CAN_lib à Arduino IDE.

---

3. Abra o firmware

Abra:

ARDUINO-UART.ino

---

4. Selecione a placa

Escolha o modelo de Arduino compatível com "Serial1".

Para um Arduino Mega:

Arduino Mega 2560

---

5. Configure a conexão

Confira:

Serial1 → Speeduino
D10     → MCP2515 CS
D13     → 2-Step
D4      → Start

---

6. Verifique o MCP2515

O código está configurado para:

CAN = 1 Mbps
Crystal = 8 MHz

Portanto, o módulo MCP2515 utilizado deve possuir cristal compatível com a configuração:

MCP_8MHZ

---

7. Faça o upload

Conecte o Arduino ao computador e faça o upload do firmware.

---

8. Abra o Monitor Serial

Configure:

115200 bps

Na inicialização normal deverá ser apresentada a mensagem:

FCAN ONLINE

---

🧪 Checklist de Teste

Antes da instalação no veículo, recomenda-se validar:

- [ ] Arduino inicializando
- [ ] MCP2515 inicializando
- [ ] Mensagem "FCAN ONLINE"
- [ ] Speeduino conectada à "Serial1"
- [ ] Comunicação em 115200 bps
- [ ] Comando "'n'" sendo enviado
- [ ] Pacote recebido corretamente
- [ ] RPM sendo interpretado
- [ ] Avanço sendo interpretado
- [ ] TPS sendo calculado
- [ ] MAP sendo interpretado
- [ ] ECT sendo calculado
- [ ] Battery sendo interpretada
- [ ] Duty Cycle sendo calculado
- [ ] Payload FCAN transmitido
- [ ] Multiframe funcionando
- [ ] Comandos CAN sendo recebidos
- [ ] 2-Step funcionando
- [ ] Start funcionando
- [ ] Timeout de 100 ms funcionando

---

⚠️ Considerações Elétricas

Os pinos:

D13 → 2-Step
D4  → Start

são saídas digitais do microcontrolador.

Eles não devem ser utilizados diretamente para acionar cargas automotivas de potência.

Utilize um estágio de acionamento adequado, como:

- transistor;
- MOSFET;
- driver;
- relé;
- circuito de isolamento.

A interface elétrica deve ser dimensionada de acordo com a carga e com o ambiente automotivo.

---

⚠️ Segurança

O firmware possui um mecanismo de segurança específico para a saída de partida através do timeout de 100 ms.

Mesmo assim, o sistema deve ser considerado um projeto de desenvolvimento, pesquisa e experimentação.

Antes da utilização em veículo, valide cuidadosamente:

- alimentação;
- aterramento;
- níveis lógicos;
- comunicação UART;
- comunicação CAN;
- terminação CAN;
- pinagem;
- acionamento das saídas;
- comportamento após reset;
- perda de comunicação com a Speeduino;
- perda de comunicação CAN;
- funcionamento do timeout de Start.

<strong>Funções relacionadas à partida do motor devem ser testadas inicialmente em condições controladas e seguras.</strong>

---

📂 Estrutura do Diretório

ARDUINO-UART/
│
├── ARDUINO-UART.ino
└── README.md

---

🔗 Navegação

<div align="center"><a href="../README.md"><strong>⬅ Voltar para o README principal do SPD CAN</strong>

</a></div>---

⚖️ Aviso Legal

Esta implementação faz parte do projeto independente SPD CAN.

O firmware é destinado a estudo, pesquisa, desenvolvimento e experimentação de sistemas embarcados e comunicação automotiva.

Este projeto não possui afiliação, patrocínio, homologação ou vínculo comercial com fabricantes de módulos eletrônicos, marcas automotivas ou sistemistas.

O uso, instalação, modificação ou aplicação deste firmware é de responsabilidade exclusiva do usuário.

---

<div align="center"><strong>SPD CAN</strong>

<br>Speeduino Telemetry • UART • Arduino • MCP2515 • CAN

<br><br>

<strong>ARDUINO-UART</strong>

</div>