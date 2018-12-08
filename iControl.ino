#include <Arduino.h>

#include "CmdParser.hpp"
#include "CmdBuffer.hpp"

CmdBuffer<128> cmdBuffer;
CmdParser cmdParser;

#define ERROR_SERIAL_PARSER -1

#define TMP A3
#define MQ5 A4
#define MQ9 A5

#define RELAY_A 2
#define RELAY_B 6

void checkCommand(void);
void changeRelay(int relay, int status);
void getStatus(float *temp, float *mq5, float *mq9);

void setup() {
    Serial.begin(115200);

    Serial.println("Started");

    pinMode(RELAY_A, OUTPUT);
    pinMode(RELAY_B, OUTPUT);

    digitalWrite(RELAY_A, LOW);
    digitalWrite(RELAY_B, LOW);
}

void loop() {
    checkCommand();
}

void changeRelay(int relay, int status) {
    digitalWrite((uint8_t)relay, (uint8_t)status);
}

void getStatus(float *temp, float *mq5, float *mq9) {
    int raw_temp = analogRead(TMP);
    int raw_mq5  = analogRead(MQ5);
    int raw_mq9  = analogRead(MQ9);

    *temp = (raw_temp / 1024.0) * 500;

    float sensor_volt;
    float RS_gas; // Get value of RS in a GAS
    float ratio; // Get ratio RS_GAS/RS_air
    int sensorValue = analogRead(A0);
    sensor_volt=(float)sensorValue/1024*5.0;
    RS_gas = (5.0-sensor_volt)/sensor_volt;
    ratio = RS_gas/1.42;

    *mq5 = ratio;

    float sensor_volt_2;
    float RS_gas_2; // Get value of RS in a GAS
    float ratio_2; // Get ratio RS_GAS/RS_air
    int sensorValue_2 = analogRead(A0);
    sensor_volt_2=(float)sensorValue_2/1024*5.0;
    RS_gas = (5.0-sensor_volt_2)/sensor_volt_2;

    ratio_2 = RS_gas/0.80;

    *mq9 = ratio_2;

}

void checkCommand(void) {
    int is_received = cmdBuffer.readFromSerial(&Serial, 1000);

    if (is_received) {
        if (cmdParser.parseCmd(cmdBuffer.getStringFromBuffer()) != CMDPARSER_ERROR) {
            String current_cmd = cmdParser.getCommand();
            if (cmdParser.equalCommand_P(PSTR("LAMP_DESK_ON"))) {
                changeRelay(RELAY_A, HIGH);
            } else if (cmdParser.equalCommand_P(PSTR("LAMP_DESK_OFF"))) {
                changeRelay(RELAY_A, LOW);
            } else if (cmdParser.equalCommand_P(PSTR("LAMP_ROOM_ON"))) {
                changeRelay(RELAY_B, HIGH);
            } else if (cmdParser.equalCommand_P(PSTR("LAMP_ROOM_OFF"))) {
                changeRelay(RELAY_B, LOW);
            }  else if (cmdParser.equalCommand_P(PSTR("GET_CONTAMINANTS"))) {
                float temp = 0.0, mq5 = 0.0, mq9 = 0.0;
                getStatus(&temp, &mq5, &mq9);

                char reply[50];
                sprintf(reply, "TEMP=%.2f/MQ5=%.2f/MQ9=%.2f/", temp, mq5, mq9);

                Serial.println(reply);

            }
        }
    }
}