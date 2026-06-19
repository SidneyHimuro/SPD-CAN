#include <SPI.h>
#include <mcp_can.h>

#define CAN_CS 10
#define PIN_2STEP_OUT 13
#define PIN_START_OUT 4

MCP_CAN CAN(CAN_CS);

//====================================================
// FCAN
//====================================================

const uint16_t PRODUCT_ID = 0x5020;
const uint8_t DATAFIELD_ID = 0x02;
const uint16_t MESSAGE_ID = 0x02FF;

uint32_t canID;

//====================================================
// Botão 2-Step recebido via CAN
//====================================================

const uint32_t TARGET_ID = 0x923013FF;

bool twoStepButton = false;

//====================================================
// Botão Start recebido via CAN
//====================================================

bool startButton   = false;
unsigned long lastStartMessage = 0;
const unsigned long START_TIMEOUT = 100;

//====================================================
// Secondary Serial Speeduino
//====================================================

const char REQUEST_COMMAND = 'n';

const int BUFFER_SIZE = 150;

uint8_t dataBuffer[BUFFER_SIZE];

int bytesReceived = 0;

unsigned long lastRequestTime = 0;

const unsigned long requestInterval = 50; // 20Hz

//====================================================

void addMeasure(uint8_t *buffer,
                int &idx,
                uint16_t measureID,
                int16_t value)
{
    buffer[idx++] = highByte(measureID);
    buffer[idx++] = lowByte(measureID);

    buffer[idx++] = highByte(value);
    buffer[idx++] = lowByte(value);
}

//====================================================

void sendFCAN(uint8_t *payload,
              uint16_t payloadLen)
{
    byte frame[8];

    frame[0] = 0x00;
    frame[1] = (payloadLen >> 8) & 0x0F;
    frame[2] = payloadLen & 0xFF;

    memcpy(&frame[3], payload, 5);

    CAN.sendMsgBuf(canID, 1, 8, frame);

    uint16_t sent = 5;
    uint8_t seg = 1;

    while(sent < payloadLen)
    {
        memset(frame, 0, sizeof(frame));

        frame[0] = seg++;

        uint8_t count = min((uint16_t)7,
                            (uint16_t)(payloadLen - sent));

        memcpy(&frame[1], &payload[sent], count);

        CAN.sendMsgBuf(canID, 1, 8, frame);

        sent += count;

        delayMicroseconds(200);
    }
}

//====================================================
// Recebe botão da Nano Pro
//====================================================

void readButtonCAN()
{
    while (CAN.checkReceive() == CAN_MSGAVAIL)
    {
        unsigned long rxId;
        byte len;
        byte rxBuf[8];

        CAN.readMsgBuf(&rxId, &len, rxBuf);

        if (rxId != TARGET_ID || len < 5)
            continue;

        //--------------------------------------------------
        // 2-Step Button
        //--------------------------------------------------

        if (rxBuf[0] == 0xFF &&
            rxBuf[1] == 0x00 &&
            rxBuf[2] == 0xEC &&
            rxBuf[3] == 0x00)
        {
            twoStepButton = (rxBuf[4] == 0x01);

            digitalWrite(
                PIN_2STEP_OUT,
                twoStepButton ? HIGH : LOW
            );
        }

        //--------------------------------------------------
        // Start Button
        //--------------------------------------------------

if (rxBuf[0] == 0xFF &&
    rxBuf[1] == 0x00 &&
    rxBuf[2] == 0xFE &&
    rxBuf[3] == 0x00)
{
    if (rxBuf[4] == 0x01)
    {
        lastStartMessage = millis();
        digitalWrite(PIN_START_OUT, HIGH);
    }
    else
    {
        digitalWrite(PIN_START_OUT, LOW);
    }
}
}
}

//====================================================

void setup()
{
    Serial.begin(115200);

    Serial1.begin(115200);

    pinMode(PIN_2STEP_OUT, OUTPUT);
    digitalWrite(PIN_2STEP_OUT, LOW);
      pinMode(PIN_START_OUT, OUTPUT);
    digitalWrite(PIN_START_OUT, LOW);

    while(CAN.begin(MCP_ANY,
                    CAN_1000KBPS,
                    MCP_8MHZ) != CAN_OK)
    {
        Serial.println("CAN FAIL");
        delay(1000);
    }

    CAN.setMode(MCP_NORMAL);

    canID =
        ((uint32_t)PRODUCT_ID << 14)
        | ((uint32_t)DATAFIELD_ID << 11)
        | MESSAGE_ID;

    Serial.println("FCAN ONLINE");
}

//====================================================

void loop()
{
    readButtonCAN();

    unsigned long currentTime = millis();

    //--------------------------------------------------
    // Solicita pacote Secondary Serial
    //--------------------------------------------------

    if(currentTime - lastRequestTime >= requestInterval)
    {
        lastRequestTime = currentTime;

        while(Serial1.available())
            Serial1.read();

        Serial1.write(REQUEST_COMMAND);

        bytesReceived = 0;
    }

    
    if (millis() - lastStartMessage > START_TIMEOUT)
{
    digitalWrite(PIN_START_OUT, LOW);
}
    
    //--------------------------------------------------
    // Recebe pacote
    //--------------------------------------------------

    while(Serial1.available())
    {
        if(bytesReceived < BUFFER_SIZE)
        {
            dataBuffer[bytesReceived++] =
                Serial1.read();
        }
        else
        {
            Serial1.read();
        }
    }

    //--------------------------------------------------
    // Processa pacote
    //--------------------------------------------------

    if(bytesReceived > 0 &&
       millis() - lastRequestTime > 20)
    {
        processSpeeduinoData();

        bytesReceived = 0;
    }
}

//====================================================

void processSpeeduinoData()
{
    if(bytesReceived < 80)
        return;

    //--------------------------------------------------
    // RPM
    //--------------------------------------------------

    uint16_t rpm =
        (dataBuffer[15] << 8) |
        dataBuffer[14];

    //--------------------------------------------------
    // ADV
    //--------------------------------------------------

    uint8_t advance = dataBuffer[8];

    //--------------------------------------------------
    // MAP
    //--------------------------------------------------

    uint16_t map =
        (dataBuffer[5] << 8) |
        dataBuffer[4];

    //--------------------------------------------------
    // TPS
    //--------------------------------------------------

    float tps =
        (dataBuffer[25] * 100L) / 200.0;

    //--------------------------------------------------
    // ECT
    //--------------------------------------------------

    int ect =
        (int)dataBuffer[7] - 40;

    //--------------------------------------------------
    // PW
    //--------------------------------------------------

    uint16_t pwRaw =
        (dataBuffer[77] << 8) |
        dataBuffer[76];

    float pw = pwRaw / 1000.0;

    float dutyCycle = 0.0;

    if (rpm > 0)
    {
        dutyCycle = (pw * rpm) / 600.0;
    }

    //--------------------------------------------------
    // BAT
    //--------------------------------------------------

    int batt = dataBuffer[9];

    //--------------------------------------------------
    // Conversões FCAN
    //--------------------------------------------------

    int rpmFc = rpm;

    int advFc = advance * 10;

    int tpsFc = tps * 10;

    int mapFc = map * 10;

    int ectFc = ect * 10;

    int battFc = batt * 10;

    int pwFc = dutyCycle * 10;

    //--------------------------------------------------
    // FCAN
    //--------------------------------------------------

    uint8_t payload[64];

    int idx = 0;

    addMeasure(payload, idx, 0x0084, rpmFc);
    addMeasure(payload, idx, 0x008E, advFc);
    addMeasure(payload, idx, 0x0002, tpsFc);
    addMeasure(payload, idx, 0x0004, mapFc);
    addMeasure(payload, idx, 0x0008, ectFc);
    addMeasure(payload, idx, 0x0012, battFc);
    addMeasure(payload, idx, 0x008A, pwFc);

    sendFCAN(payload, idx);
}