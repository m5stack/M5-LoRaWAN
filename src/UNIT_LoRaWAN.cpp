#include "UNIT_LoRaWAN.h"

void UNIT_LoRaWAN::Init(HardwareSerial *serial, uint8_t RX, uint8_t TX) {
    _serial = serial;
    _serial->begin(115200, SERIAL_8N1, RX, TX);
    _serial->flush();
}

bool UNIT_LoRaWAN::checkDeviceConnect() {
    String restr;
    writeCMD("AT+CGMI?\r\n");
    restr = waitMsg(500);
    if (restr.indexOf("OK") == -1) {
        return false;
    } else {
        return true;
    }
}

bool UNIT_LoRaWAN::checkJoinStatus() {
    String restr;
    writeCMD("AT+CSTATUS?\r\n");
    restr = waitMsg(500);
    if (restr.indexOf("+CSTATUS:") != -1) {
        if (restr.indexOf("03") != -1 || restr.indexOf("07") != -1 ||
            restr.indexOf("08") != -1) {
            return true;
        } else {
            return false;
        }
    } else {
        return false;
    }
}

String UNIT_LoRaWAN::waitMsg(unsigned long time) {
    String restr;
    unsigned long start = millis();
    while (1) {
        if (_serial->available() || (millis() - start) < time) {
            String str = _serial->readString();
            restr += str;
        } else {
            break;
        }
    }
    return restr;
}

void UNIT_LoRaWAN::writeCMD(String command) {
    _serial->print(command);
    delay(100);
}

void UNIT_LoRaWAN::sendMsg(uint8_t confirm, uint8_t nbtrials, size_t length,
                           String data) {
    String cmd = "AT+DTRX=" + String(confirm) + ',' + String(nbtrials) + ',' +
                 String(length) + ',' + data + "\r\n";
    writeCMD(cmd);
}

String UNIT_LoRaWAN::receiveMsg() {
    String restr = waitMsg(2000);
    if (restr.indexOf("OK+RECV:") != -1 && restr.indexOf("02,00,00") == -1) {
        String data = restr.substring(restr.indexOf("OK+RECV:") + 17, -2);
        return data;
    } else {
        return "";
    }
}

void UNIT_LoRaWAN::configOTTA(String device_eui, String app_eui, String app_key,
                              String ul_dl_mode) {
    writeCMD("AT+CJOINMODE=0\r\n");
    writeCMD("AT+CDEVEUI=" + device_eui + "\r\n");
    writeCMD("AT+CAPPEUI=" + app_eui + "\r\n");
    writeCMD("AT+CAPPKEY=" + app_key + "\r\n");
    writeCMD("AT+CULDLMODE=" + ul_dl_mode + "\r\n");
}

void UNIT_LoRaWAN::configABP(String device_addr, String app_skey,
                             String net_skey, String ul_dl_mode) {
    writeCMD("AT+CJOINMODE=1\r\n");
    writeCMD("AT+CDEVADDR=" + device_addr + "\r\n");
    writeCMD("AT+CAPPSKEY=" + app_skey + "\r\n");
    writeCMD("AT+CNWKSKEY=" + net_skey + "\r\n");
    writeCMD("AT+CULDLMODE=" + ul_dl_mode + "\r\n");
}

void UNIT_LoRaWAN::setClass(String mode) {
    writeCMD("AT+CCLASS=" + mode + "\r\n");
}

void UNIT_LoRaWAN::setRxWindow(String freq) {
    writeCMD("AT+CRXP=0,0," + freq + "\r\n");
}

void UNIT_LoRaWAN::setFreqMask(String mask) {
    writeCMD("AT+CFREQBANDMASK=" + mask + "\r\n");
}

void UNIT_LoRaWAN::startJoin() {
    writeCMD("AT+CJOIN=1,0,10,8\r\n");
}
