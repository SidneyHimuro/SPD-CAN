# SPD CAN

O **SPD CAN** é um firmware de gateway/bridge de comunicação automotiva de alta performance. O projeto atua como um tradutor síncrono bidirecional: ele realiza requisições de telemetria via interface serial secundária (UART) para uma Unidade de Controle do Motor (ECU) de código aberto e converte esses dados estruturalmente para transmissão em tempo real em um barramento multiplexado CAN (Controller Area Network).

Este software foi desenhado com foco em portabilidade, eficiência de barramento e total interoperabilidade com displays, painéis digitais e registradores de dados (dataloggers) que operam com protocolos de recepção multiframe baseados em identificadores estendidos.

---

## 🚀 Funcionalidades

* **Varredura Ativa a 20Hz:** Solicitação cíclica e controlada de pacotes seriais com intervalos precisos de 50ms para evitar saturação da interface da ECU.
* **Processamento e Escalonamento:** Extração nativa de variáveis binárias e conversão matemática para o formato de ponto fixo exigido pelo barramento receptor.
* **Transmissão Multiframe Automatizada:** Algoritmo embarcado de fragmentação de payloads complexos. Dados superiores a 5 bytes são automaticamente segmentados e sequenciados em múltiplos quadros CAN de 8 bytes com atraso de proteção entre pacotes ($200\,\mu\text{s}$).
* **Interface de Diagnóstico:** Saída de depuração via Serial nativa (USB) a 115200 bps para validação e monitoramento das grandezas em tempo real.

---

## 🛠️ Especificações Técnicas de Hardware

O firmware é compatível com plataformas de desenvolvimento do ecossistema Arduino que possuam suporte a múltiplos barramentos seriais em hardware (HardwareSerial) e interface com controladores CAN baseados no ecossistema SPI.

### Configuração do Barramento CAN
* **Velocidade da Rede:** 1000 Kbps (1 Mbps)
* **Frequência do Cristal do Controlador:** 8 MHz
* **Identificadores Utilizados:** Modo Estendido (29 bits)
* **Pino Padrão de Chip Select (CS):** Digital 10 (configurável no código)

### Configuração da Interface Serial (UART)
* **Baud Rate (ECU & Debug):** 115200 bps
* **Payload Base de Entrada:** Buffer dinâmico com capacidade de até 150 bytes.

---

## 📡 Arquitetura do Protocolo e Mapeamento de Canais

A estrutura de empacotamento da rede CAN utiliza como referência especificações técnicas de protocolos industriais e automotivos de domínio público disponíveis na internet. Os identificadores das mensagens são construídos dinamicamente via operadores lógicos de deslocamento de bits (*Bitwise*) a partir de constantes pré-definidas (`0x5020`, `0x02` e `0x02FF`).

As grandezas convertidas e seus respectivos identificadores hexadecimais de canal (Measure IDs) seguem a tabela abaixo:

| Parâmetro Técnico | ID do Canal | Escalonamento | Unidade Original |
| :--- | :---: | :---: | :--- |
| **Rotação do Motor (RPM)** | `0x0084` | $1:1$ | rpm |
| **Ponto / Avanço de Ignição** | `0x008E` | $\times 10$ | Graus (°) |
| **Posição de Borboleta (TPS)** | `0x0002` | $\times 10$ | Porcentagem (%) |
| **Pressão Absoluta (MAP)** | `0x0004` | $\times 10$ | kPa |
| **Temperatura do Motor (ECT)** | `0x0008` | $\times 10$ | Celsius (°C) |
| **Tensão da Bateria (BAT)** | `0x0012` | $\times 10$ | Volts (V) |
| **Tempo de Injeção / Duty Cycle** | `0x008A` | $\times 10$ | Porcentagem (%) |

---

## 🧬 Lógica de Segmentação (Multiframe)

Para a transmissão completa do bloco de dados sem perda de sincronismo, o firmware empacota o payload estruturado em sub-quadros sequenciais:

1. **Quadro de Abertura (Índice `0x00`):** Informa nos bytes iniciais o comprimento total do payload e anexa os primeiros 5 bytes de dados úteis.
2. **Quadros Consecutivos (Índices `0x01` em diante):** Carregam o índice do segmento no byte inicial seguido por até 7 bytes de dados sequenciais até que todo o bloco seja esgotado.

---

## 🔧 Como Utilizar

1. Certifique-se de incluir no seu gerenciador de bibliotecas a biblioteca padrão para controladores CAN SPI (ex: `mcp_can`).
2. Conecte o pino **TX1/RX1** do seu microcontrolador à porta de comunicação secundária da sua unidade de processamento.
3. Altere o pino `#define CAN_CS 10` no arquivo principal caso utilize uma pinagem diferente para o barramento SPI.
4. Faça o upload do firmware e monitore as conversões através do Monitor Serial a 115200 bps.

---

## ⚖️ Aviso Legal / Disclaimer

Este é um projeto independente, de código aberto, com fins estritamente de estudo, pesquisa e desenvolvimento de sistemas embarcados.

* **Isenção de Vínculo:** Este projeto foi desenvolvido de forma 100% autônoma e não possui qualquer tipo de afiliação, patrocínio, homologação ou vínculo comercial com fabricantes de módulos eletrônicos, marcas automotivas ou sistemistas.
* **Propriedade de Terceiros:** Quaisquer mapeamentos de canais, constantes numéricas ou arquiteturas de rede implícitas baseiam-se em dados factuais de mercado e especificações públicas de engenharia, servindo única e exclusivamente para garantir o direito de interoperabilidade de sistemas (comunicação entre dispositivos distintos), conforme amparado pelas legislações de propriedade intelectual e direitos de software.
* **Responsabilidade:** O uso, aplicação ou modificação deste firmware em qualquer ambiente de testes ou automotivo é de total e exclusiva responsabilidade do usuário final.
