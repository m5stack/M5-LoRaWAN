/*
*******************************************************************************
* Copyright (c) 2022 by M5Stack
*                  Equipped with Atom-Lite/Matrix sample source code
*                          配套  Atom-Lite/Matrix 示例源代码
* Visit for more information: https://docs.m5stack.com/en/unit/lorawan470
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/unit/lorawan470
*
* Product: LoRaWAN470.
* Date: 2022/8/19
*******************************************************************************
  Please connect to Port C,请连接端口C
*/

#include "M5Atom.h"
#include "M5_LoRaWAN.h"
#include "freertos/queue.h"

M5_LoRaWAN LoRaWAN;

String response;

void setup() {
    M5.begin(true, false, true);
    M5.dis.fillpix(0xffff00);
    LoRaWAN.Init(&Serial2, 32, 26);
    delay(100);

    Serial.println("Module Connect.....");
    while (!LoRaWAN.checkDeviceConnect())
        ;
    M5.dis.fillpix(0x00ff00);
    LoRaWAN.writeCMD("AT?\r\n");
    delay(100);
    Serial2.flush();

    // Disable Log Information
    LoRaWAN.writeCMD("AT+ILOGLVL=0\r\n");
    // Enable  Log Information
    LoRaWAN.writeCMD("AT+CSAVE\r\n");
    LoRaWAN.writeCMD("AT+IREBOOT=0\r\n");
    Serial.println("LoraWan Rebooting");
    delay(1000);

    Serial.println("LoraWan config");
    // Set Join Mode OTAA.
    LoRaWAN.configOTTA("00bb9da5b97addf1",                  // Device EUI
                       "0000000000000000",                  // APP EUI
                       "27dfe264ca33ac1957c005eb48ba4721",  // APP KEY
                       "2"  // Upload Download Mode
    );
    response = LoRaWAN.waitMsg(1000);
    Serial.println(response);
    // Set ClassC mode
    LoRaWAN.setClass("2");
    LoRaWAN.writeCMD("AT+CWORKMODE=2\r\n");

    // LoRaWAN470
    LoRaWAN.setRxWindow("505300000");

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
    LoRaWAN.setFreqMask("0400");

    // RX Freq
    // 506.7 (RX1)
    // 506.9 (RX1)
    // 507.1 (RX1)
    // 507.3 (RX1)
    // 507.5 (RX1)
    // 507.7 (RX1)
    // 507.9 (RX1)
    // 508.1 (RX1)
    // 505.3 (RX2)| 505300000
    LoRaWAN.startJoin();
}

enum systemstate {
    kIdel = 0,
    kJoined,
    kSending,
    kWaitSend,
    kEnd,
};
int system_fsm = kIdel;

int loraWanSendNUM = -1;
int loraWanSendCNT = -1;

String waitRevice() {
    String recvStr;
    do {
        recvStr = Serial2.readStringUntil('\n');
    } while (recvStr.length() == 0);
    Serial.println(recvStr);
    return recvStr;
}

void loop() {
    String recvStr = waitRevice();
    // String recvStr = LoRaWAN.waitMsg(2000);
    if (recvStr.indexOf("+CJOIN:") != -1) {
        if (recvStr.indexOf("OK") != -1) {
            Serial.printf("LoraWan JOIN");
            system_fsm = kJoined;
        } else {
            Serial.printf("LoraWan JOIN FAIL");
            system_fsm = kIdel;
        }
    } else if (recvStr.indexOf("OK+RECV") != -1) {
        if (system_fsm == kJoined) {
            system_fsm = kSending;
        } else if (system_fsm == kWaitSend) {
            system_fsm = kSending;
            char strbuff[128];
            if ((loraWanSendCNT < 5) && (loraWanSendNUM == 8)) {
                sprintf(strbuff, "TSET OK CNT: %d", loraWanSendCNT);
                Serial.print(strbuff);
            } else {
                sprintf(strbuff, "FAILD NUM:%d CNT:%d", loraWanSendNUM,
                        loraWanSendCNT);
                Serial.print(strbuff);
            }
        }
    } else if (recvStr.indexOf("OK+SEND") != -1) {
        String snednum = recvStr.substring(8);
        Serial.printf(" [ INFO ] SEND NUM %s \r\n", snednum.c_str());
        loraWanSendNUM = snednum.toInt();
    } else if (recvStr.indexOf("OK+SENT") != -1) {
        String snedcnt = recvStr.substring(8);
        Serial.printf(" [ INFO ] SEND CNT %s \r\n", snedcnt.c_str());
        loraWanSendCNT = snedcnt.toInt();
    }

    if (system_fsm == kSending) {
        Serial.println("LoraWan Sending");
        LoRaWAN.writeCMD("AT+DTRX=1,8,8,4655434b20535443\r\n");
        system_fsm = kWaitSend;
    }
    delay(10);
}