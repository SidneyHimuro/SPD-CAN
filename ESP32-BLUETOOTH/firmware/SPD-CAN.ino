/*
 * ============================================================================
 * Projeto: Speeduino to FTCAN Gateway (SPD-CAN)
 * Hardware: ESP32 + MCP2515 (SPI) + Bluetooth Serial (HC-06)
 * Descrição: Lê telemetria da Speeduino (Serial1) via Bluetooth (Comando 'A') 
 *            e transmite via rede CAN no formato FTCAN para dashboards.
 * ============================================================================
 */

#include <SPI.h>
#include <mcp_can.h>
#include "BluetoothSerial.h"

// ============================================================================
// CONFIGURAÇÕES DE HARDWARE & PINOS
// ============================================================================
#define CAN_CS_PIN        5
#define PIN_2STEP_OUT     13
#define PIN_START_OUT     4

MCP_CAN CAN(CAN_CS_PIN);
BluetoothSerial SerialBT;

// MAC Address do módulo HC-06 conectado à Speeduino
uint8_t hc06Address[6] = {0x98, 0xD3, 0x71, 0xF9, 0xA4, 0xEE};

// ============================================================================
// CONFIGURAÇÕES PROTOCOLO FTCAN
// ============================================================================
const uint16_t PRODUCT_ID   = 0x5020;
const uint8_t  DATAFIELD_ID = 0x02;
const uint16_t MESSAGE_ID   = 0x02FF;
uint32_t canID;

// ID de botões recebidos da rede CAN (Nano PRO)
const uint32_t TARGET_CAN_ID = 0x923013FF;

bool twoStepButton = false;
bool startButton   = false;
unsigned long lastStartMessage = 0;
const unsigned long START_TIMEOUT = 100; // ms

// ============================================================================
// BUFFER SERIAL SPEEDUINO (COMANDO 'A' = 130 BYTES)
// ============================================================================
const char REQUEST_COMMAND = 'A';
const int EXPECTED_BYTES   = 130;
const int BUFFER_SIZE      = 160;

uint8_t dataBuffer[BUFFER_SIZE];
int bytesReceived = 0;

unsigned long lastRequestTime = 0;
const unsigned long REQUEST_INTERVAL = 50; // 20Hz (50ms)

// ============================================================================
// FUNÇÕES AUXILIARES CAN
// ============================================================================
void addMeasure(uint8_t *buffer, int &idx, uint16_t measureID, int16_t value) {
    buffer[idx++] = highByte(measureID);
    buffer[idx++] = lowByte(measureID);
    buffer[idx++] = highByte(value);
    buffer[idx++] = lowByte(value);
}

void sendFCAN(uint8_t *payload, uint16_t payloadLen) {
    byte frame[8];

    frame[0] = 0x00;
    frame[1] = (payloadLen >> 8) & 0x0F;
    frame[2] = payloadLen & 0xFF;
    memcpy(&frame[3], payload, 5);

    if (CAN.sendMsgBuf(canID, 1, 8, frame) != CAN_OK) {
        return;
    }

    uint16_t sent = 5;
    uint8_t seg = 1;

    while (sent < payloadLen) {
        memset(frame, 0, sizeof(frame));
        frame[0] = seg++;
        uint8_t count = min((uint16_t)7, (uint16_t)(payloadLen - sent));
        memcpy(&frame[1], &payload[sent], count);
        CAN.sendMsgBuf(canID, 1, 8, frame);
        sent += count;
        delayMicroseconds(200);
    }
}

void readButtonCAN() {
    while (CAN.checkReceive() == CAN_MSGAVAIL) {
        unsigned long rxId;
        byte len;
        byte rxBuf[8];

        CAN.readMsgBuf(&rxId, &len, rxBuf);

        if (rxId != TARGET_CAN_ID || len < 5) continue;

        // Leitura Botão 2-Step
        if (rxBuf[0] == 0xFF && rxBuf[1] == 0x00 && rxBuf[2] == 0xEC && rxBuf[3] == 0x00) {
            twoStepButton = (rxBuf[4] == 0x01);
            digitalWrite(PIN_2STEP_OUT, twoStepButton ? HIGH : LOW);
        }

        // Leitura Botão Partida (Start)
        if (rxBuf[0] == 0xFF && rxBuf[1] == 0x00 && rxBuf[2] == 0xFE && rxBuf[3] == 0x00) {
            if (rxBuf[4] == 0x01) {
                lastStartMessage = millis();
                digitalWrite(PIN_START_OUT, HIGH);
            } else {
                digitalWrite(PIN_START_OUT, LOW);
            }
        }
    }
}

// ============================================================================
// PROCESSAMENTO DA TELEMETRIA SPEEDUINO
// ============================================================================
void processSpeeduinoData() {
    if (bytesReceived < EXPECTED_BYTES) return;

    // Extração de variáveis do frame de 130 bytes
    uint16_t rpm     = (dataBuffer[15] << 8) | dataBuffer[14];
    uint8_t  advance = dataBuffer[8];
    uint16_t mapRaw  = (dataBuffer[5] << 8) | dataBuffer[4]; 
    float    tps     = (dataBuffer[24] * 100.0) / 255.0;
    int      ect     = (int)dataBuffer[7] - 40;

    uint16_t pwRaw   = (dataBuffer[77] << 8) | dataBuffer[76];
    float    pw      = pwRaw / 1000.0;
    float dutyCycle  = (rpm > 0) ? ((pw * rpm) / 600.0) : 0.0;

    int     batt      = dataBuffer[9];
    uint8_t fuelPress = dataBuffer[55];
    uint8_t oilPress  = dataBuffer[56];

    float mapBarAbs = mapRaw / 100.0; 

    // Telemetria Monitor Serial
    Serial.printf("[ECU OK] RPM: %d | MAP: %.2f Bar | TPS: %.1f%% | ECT: %d C | BAT: %.1fV\n", 
                  rpm, mapBarAbs, tps, ect, batt / 10.0);

    // Escalonamento FTCAN
    int rpmFc  = rpm;
    int advFc  = advance * 10;
    int tpsFc  = tps * 10;
    int mapFc  = (int)(mapBarAbs * 100.0); 
    int ectFc  = ect * 10;
    int battFc = batt * 10;
    int pwFc   = dutyCycle * 10;
    int fuelFc = fuelPress * 10;
    int oilFc  = oilPress * 10;

    // Montagem Payload FTCAN
    uint8_t payload[64];
    int idx = 0;

    addMeasure(payload, idx, 0x0084, rpmFc);
    addMeasure(payload, idx, 0x008E, advFc);
    addMeasure(payload, idx, 0x0002, tpsFc);
    addMeasure(payload, idx, 0x0004, mapFc);
    addMeasure(payload, idx, 0x0008, ectFc);
    addMeasure(payload, idx, 0x0012, battFc);
    addMeasure(payload, idx, 0x008A, pwFc);
    addMeasure(payload, idx, 0x000E, fuelFc);
    addMeasure(payload, idx, 0x0010, oilFc);

    sendFCAN(payload, idx);
}

// ============================================================================
// SETUP
// ============================================================================
void setup() {
    Serial.begin(115200);

    pinMode(PIN_2STEP_OUT, OUTPUT);
    digitalWrite(PIN_2STEP_OUT, LOW);

    pinMode(PIN_START_OUT, OUTPUT);
    digitalWrite(PIN_START_OUT, LOW);

    SPI.begin(18, 19, 23, CAN_CS_PIN);
    
    while (CAN.begin(MCP_ANY, CAN_1000KBPS, MCP_8MHZ) != CAN_OK) {
        Serial.println("[ERRO SPI] MCP2515 nao respondeu!");
        delay(1000);
    }
    Serial.println("[MCP2515] Inicializado em 1000kbps (8MHz) OK!");
    CAN.setMode(MCP_NORMAL);

    canID = ((uint32_t)PRODUCT_ID << 14) |
            ((uint32_t)DATAFIELD_ID << 11) |
            MESSAGE_ID;

    SerialBT.begin("SPD_CAN_Gateway", true);
    SerialBT.setPin("1234", 4); 

    SerialBT.disconnect();
    delay(500);

    Serial.println("Conectando ao Bluetooth HC-06...");
    if (SerialBT.connect(hc06Address)) {
        Serial.println("--> Bluetooth Conectado!");
    }
}

// ============================================================================
// LOOP PRINCIPAL
// ============================================================================
void loop() {
    // Reconexão Bluetooth Automática
    if (!SerialBT.connected(10)) {
        static unsigned long lastRetry = 0;
        if (millis() - lastRetry > 4000) {
            lastRetry = millis();
            Serial.println("Reconectando ao HC-06...");
            SerialBT.disconnect();
            delay(100);
            SerialBT.connect(hc06Address);
        }
    }

    readButtonCAN();

    unsigned long currentTime = millis();

    // Requisição periódica do comando 'A'
    if (SerialBT.connected() && (currentTime - lastRequestTime >= REQUEST_INTERVAL)) {
        lastRequestTime = currentTime;
        while (SerialBT.available()) SerialBT.read(); // Limpa lixo residual
        SerialBT.write(REQUEST_COMMAND);
        bytesReceived = 0;
    }

    if (millis() - lastStartMessage > START_TIMEOUT) {
        digitalWrite(PIN_START_OUT, LOW);
    }

    // Leitura contínua dos bytes via BT
    while (SerialBT.available()) {
        if (bytesReceived < BUFFER_SIZE) {
            dataBuffer[bytesReceived++] = SerialBT.read();
        } else {
            SerialBT.read();
        }
    }

    // Processa quando atinge 130 bytes
    if (bytesReceived >= EXPECTED_BYTES) {
        processSpeeduinoData();
        bytesReceived = 0;
    }
}
