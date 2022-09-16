/*!
 * @brief A LoRaWan communication unit From M5Stack
 * @copyright Copyright (c) 2022 by M5Stack[https://m5stack.com]
 * @Links [M5Unit-LoRaWAN470](https://docs.m5stack.com/en/unit/lorawan470)
 * @Links [M5Unit-LoRaWAN868](https://docs.m5stack.com/en/unit/lorawan868)
 * @Links [M5Unit-LoRaWAN915](https://docs.m5stack.com/en/unit/lorawan915)
 * @version  V0.0.1
 * @date  2022-08-19
 */
#ifndef _M5_LORAWAN_H_
#define _M5_LORAWAN_H_

#include <Arduino.h>
#include "pins_arduino.h"

class M5_LoRaWAN {
   private:
    HardwareSerial *_serial;
    String encodeMsg(String ret);
    String decodeMsg(String ret);

   public:
    void Init(HardwareSerial *serial = &Serial2, uint8_t RX = 16,
              uint8_t TX = 17);
    String waitMsg(unsigned long time);
    bool checkDeviceConnect();
    bool checkJoinStatus();
    void writeCMD(String command);
    void configOTTA(String device_eui, String app_eui, String app_key,
                    String ul_dl_mode);
    void configABP(String device_addr, String app_skey, String net_skey,
                   String ul_dl_mode);
    void setClass(String mode);
    void setRxWindow(String freq);
    void setFreqMask(String mask);
    void startJoin();
    void sendMsg(uint8_t confirm, uint8_t nbtrials, String data);
    String receiveMsg();
};

#endif