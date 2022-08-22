#include "M5_LoRaWAN.h"
#include "M5Stack.h"

M5_LoRaWAN LoRaWAN;
String response;

void setup() {
    M5.begin();
    LoRaWAN.Init(&Serial2, 16, 17);
    Serial.println("Module Connect.....");
    while (!LoRaWAN.checkDeviceConnect())
        ;
    Serial.println("Module Connected");
    LoRaWAN.writeCMD("AT+CRESTORE\r\n");
    // Disable Log Information
    LoRaWAN.writeCMD("AT+ILOGLVL=0\r\n");
    // Enable  Log Information
    // LoRaWAN.writeCMD("AT+ILOGLVL=5\r\n");
    LoRaWAN.writeCMD("AT+CSAVE\r\n");
    LoRaWAN.writeCMD("AT+IREBOOT=0\r\n");
    delay(1000);
    while (!LoRaWAN.checkDeviceConnect())
        ;
    Serial.println("Module Config...");
    LoRaWAN.configOTTA("00bb9da5b97addf1",                  // Device EUI
                       "0000000000000000",                  // APP EUI
                       "27dfe264ca33ac1957c005eb48ba4721",  // APP KEY
                       "2"  // Upload Download Mode
    );

    response = LoRaWAN.waitMsg(1000);
    Serial.println(response);

    // Set Class Mode
    LoRaWAN.setClass("2");

    LoRaWAN.writeCMD("AT+CWORKMODE=2\r\n");

    // LoRaWAN470
    // LoRaWAN.setRxWindow("505300000");

    // LoRaWAN868
    LoRaWAN.setRxWindow("869525000");

    // LoRaWAN915
    // LoRaWAN.setRxWindow("923300000");

    // LoRaWAN470 TX Freq
    // 486.3
    // 486.5
    // 486.7
    // 486.9
    // 487.1
    // 487.3
    // 487.5
    // 487.7
    // MARK 0000 0100 0000 0000 | 0x0400
    // LoRaWAN.setFreqMask("0400");

    // LoRaWAN868 TX Freq
    // 868.1 - SF7BW125 to SF12BW125
    // 868.3 - SF7BW125 to SF12BW125 and SF7BW250
    // 868.5 - SF7BW125 to SF12BW125
    // 867.1 - SF7BW125 to SF12BW125
    // 867.3 - SF7BW125 to SF12BW125
    // 867.5 - SF7BW125 to SF12BW125
    // 867.7 - SF7BW125 to SF12BW125
    // 867.9 - SF7BW125 to SF12BW125
    // 868.8 - FSK
    LoRaWAN.setFreqMask("0001");

    // LoRaWAN915 TX Freq
    // 902.3
    // 902.5
    // 902.7
    // 902.9
    // 903.1
    // 903.3
    // 903.5
    // 903.7
    // MARK 0000 0000 0000 0001 | 0x001
    // LoRaWAN.setFreqMask("0001");
    delay(100);
    response = LoRaWAN.waitMsg(1000);
    Serial.println(response);
    LoRaWAN.startJoin();
    Serial.println("Start Join.....");
    while (!LoRaWAN.checkJoinStatus()) {
        delay(100);
    };
    Serial.println("Join success.....");
}

void loop() {
    // send data
    Serial.println("Sending Data: ");
    LoRaWAN.sendMsg(1, 15, 7, "4d35535441434b");
    // receive data
    response = LoRaWAN.receiveMsg();
    if (response != "") {
        Serial.println("Received: ");
        Serial.println(response);
    }
    delay(3000);
}