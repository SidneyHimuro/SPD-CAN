#include <SPI.h>
#include <mcp_can.h>

#define CAN_CS 10

MCP_CAN CAN(CAN_CS);

//====================================================
// FCAN
//====================================================

const uint16_t PRODUCT_ID   = 0x5020;
const uint8_t  DATAFIELD_ID = 0x02;
const uint16_t MESSAGE_ID   = 0x02FF;

uint32_t canID;

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

    uint8_t count =
      min((uint16_t)7,
          (uint16_t)(payloadLen - sent));

    memcpy(&frame[1],
           &payload[sent],
           count);

    CAN.sendMsgBuf(canID, 1, 8, frame);

    sent += count;

    delayMicroseconds(200);
  }
}

//====================================================

void setup()
{
  Serial.begin(115200);

  Serial1.begin(115200);

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

  //-------------------------------------------------
  // ADV
  //-------------------------------------------------
  // ADV Current Advance (Byte 8) - Avanço de Ignição em Graus
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
      (dataBuffer[25] * 100L) / 200;

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
  if (rpm > 0) {
    dutyCycle = (pw * rpm) / 600.0;
  }
  //--------------------------------------------------
  // BAT
  //--------------------------------------------------

  int batt =
      dataBuffer[9];

  //--------------------------------------------------
  // Conversões FCAN
  //--------------------------------------------------

  int rpmFt  = rpm;

  int advFt = advance * 10;

  float tpsFt  = tps * 10;

  int mapFt  = map * 10;   // Nano em kPa

  int ectFt  = ect * 10;

  int battFt = batt * 10;

  float pwFt = dutyCycle * 10;

  //--------------------------------------------------
  // FCAN
  //--------------------------------------------------

  uint8_t payload[64];

  int idx = 0;

  addMeasure(payload, idx, 0x0084, rpmFt);   // RPM

  addMeasure(payload, idx, 0x008E, advFt);   // ADV

  addMeasure(payload, idx, 0x0002, tpsFt);   // TPS

  addMeasure(payload, idx, 0x0004, mapFt);   // MAP

  addMeasure(payload, idx, 0x0008, ectFt);   // ECT

  addMeasure(payload, idx, 0x0012, battFt);  // BAT

  addMeasure(payload, idx, 0x008A, pwFt);    // Inj. A

  sendFTCAN(payload, idx);

  //--------------------------------------------------
  // Debug
  //--------------------------------------------------

  static uint32_t dbg = 0;

  if(millis() - dbg > 1000)
  {
    dbg = millis();

    Serial.print("RPM=");
    Serial.print(rpm);

    Serial.print(" TPS=");
    Serial.print(tps);

    Serial.print(" MAP=");
    Serial.print(map);

    Serial.print(" ECT=");
    Serial.print(ect);

    Serial.print(" PW=");
    Serial.print(pw, 2);

    Serial.print(" BAT=");
    Serial.println(batt / 10.0);
  }
}