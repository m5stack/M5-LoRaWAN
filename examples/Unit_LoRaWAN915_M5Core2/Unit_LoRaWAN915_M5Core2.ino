/*
*******************************************************************************
* Copyright (c) 2022 by M5Stack
*                  Equipped with M5Core2 sample source code
*                          配套  M5Core2 示例源代码
* Visit for more information: https://docs.m5stack.com/en/unit/lorawan915
* 获取更多资料请访问: https://docs.m5stack.com/zh_CN/unit/lorawan915
*
* Product: LoRaWAN915.
* Date: 2022/8/19
*******************************************************************************
  Please connect to Port C,请连接端口C
*/

#include "M5Core2.h"
#include <M5GFX.h>
#include "M5_LoRaWAN.h"
#include "freertos/queue.h"

M5GFX display;
M5Canvas canvas(&display);
M5_LoRaWAN LoRaWAN;

String response;

void setup() {
    M5.begin();
    LoRaWAN.Init(&Serial2, 13, 14);
    delay(100);
    display.begin();
    display.setTextSize(2);
    canvas.setColorDepth(1);  // mono color
    canvas.createSprite(display.width(), display.height());
    canvas.setTextSize((float)canvas.width() / 160);
    canvas.setTextScroll(true);

    display.println("Module Connect.....");
    while (!LoRaWAN.checkDeviceConnect())
        ;
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
    LoRaWAN.configOTTA("d896e0ff00000240",                  // Device EUI
                       "0000000000000001",                  // APP EUI
                       "98929b92f09e2daf676d646d0f61d250",  // APP KEY
                       "2"  // Upload Download Mode
    );
    response = LoRaWAN.waitMsg(1000);
    Serial.println(response);
    // Set ClassC mode
    LoRaWAN.setClass("2");
    LoRaWAN.writeCMD("AT+CWORKMODE=2\r\n");

    // LoRaWAN915
    // TX Freq
    // 902.3
    // 902.5
    // 902.7
    // 902.9
    // 903.1
    // 903.3
    // 903.5
    // 903.7
    // MARK 0000 0000 0000 0001 | 0x001
    LoRaWAN.setFreqMask("0001");

    // RX Freq
    // 923.3 - (RX1)
    // 923.9 - (RX1)
    // 924.5 - (RX1)
    // 925.1 - (RX1)
    // 925.7 - (RX1)
    // 926.3 - (RX1)
    // 926.9 - (RX1)
    // 927.5 - (RX1)
    // 923.3 - (RX2)   | 923300000
    LoRaWAN.setRxWindow("923300000");
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
    canvas.println(recvStr);
    return recvStr;
}

void loop() {
    String recvStr = waitRevice();
    // String recvStr = LoRaWAN.waitMsg(2000);
    if (recvStr.indexOf("+CJOIN:") != -1) {
        if (recvStr.indexOf("OK") != -1) {
            canvas.printf("LoraWan JOIN");
            system_fsm = kJoined;
        } else {
            canvas.printf("LoraWan JOIN FAIL");
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
                canvas.print(strbuff);
            } else {
                sprintf(strbuff, "FAILD NUM:%d CNT:%d", loraWanSendNUM,
                        loraWanSendCNT);
                canvas.print(strbuff);
            }
        }
    } else if (recvStr.indexOf("OK+SEND") != -1) {
        String snednum = recvStr.substring(8);
        canvas.printf(" [ INFO ] SEND NUM %s \r\n", snednum.c_str());
        loraWanSendNUM = snednum.toInt();
    } else if (recvStr.indexOf("OK+SENT") != -1) {
        String snedcnt = recvStr.substring(8);
        canvas.printf(" [ INFO ] SEND CNT %s \r\n", snedcnt.c_str());
        loraWanSendCNT = snedcnt.toInt();
    }

    if (system_fsm == kSending) {
        canvas.println("LoraWan Sending");
        LoRaWAN.writeCMD("AT+DTRX=1,8,8,4655434b20535443\r\n");
        system_fsm = kWaitSend;
    }
    canvas.pushSprite(0, 0);
    delay(10);
}