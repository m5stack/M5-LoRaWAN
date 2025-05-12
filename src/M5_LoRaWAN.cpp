#include "M5_LoRaWAN.h"

/*! @brief Initialize the LoRaWAN.
 *  @param RX Receiving ports.
 *  @param TX Sending ports. */
void M5_LoRaWAN::Init(HardwareSerial* serial, uint8_t RX, uint8_t TX) {
    _serial = serial;
    _serial->begin(115200, SERIAL_8N1, RX, TX);
    _serial->flush();
}

/*! @brief Check that LoRaWAN devices are connected
 *  @return Return 1 if the connect successful, otherwise 0.. */
bool M5_LoRaWAN::checkDeviceConnect() {
    String restr;
    writeCMD("AT+CGMI?\r\n");
    restr = waitMsg(500);
    if (restr.indexOf("OK") == -1) {
        return false;
    } else {
        return true;
    }
}

/*! @brief Check the status of your network connection
 *  @return Return 1 for good connection status, otherwise 0.. */
bool M5_LoRaWAN::checkJoinStatus() {
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

/*! @brief Waiting for a period of time to receive a message
 *  @param time Waiting time (milliseconds)
 *  @return Received messages */
String M5_LoRaWAN::waitMsg(unsigned long time) {
  unsigned long start = millis();
  String str;
  while (1) {
    if ((millis() - start) > time) {
      start = millis();
      String cmd = "AT+DRX=?\r\n";
      writeCMD(cmd);
      str = _serial->readString();
      break;
    }
  }
  return str;
}

/*! @brief Send an AT command
 *  @param time AT command */
void M5_LoRaWAN::writeCMD(String command) {
    _serial->print(command);
    delay(100);
}

/*! @brief Send a message */
void M5_LoRaWAN::sendMsg(uint8_t confirm, uint8_t nbtrials, String data) {
    String encodedData = encodeMsg(data);
    String cmd = "AT+DTRX=" + String(confirm) + ',' + String(nbtrials) + ',' +
                 String(encodedData.length()) + ',' + encodedData + "\r\n";
    writeCMD(cmd);
}

/*! @brief Receive a message
 *  @return Received messages */
String M5_LoRaWAN::receiveMsg() {
    String restr = waitMsg(2000);
    if (restr.indexOf("OK+RECV:") != -1 && restr.indexOf("02,00,00") == -1) {
        String data = restr.substring(restr.indexOf("OK+RECV:") + 17, -2);
        return decodeMsg(data);
    } else {
        return "";
    }
}

/*! @brief Configuring OTAA access
 *  @param device_eui Node DevEUI
 *  @param app_eui Node AppEUI
 *  @param app_key Node AppKey
 *  @param ul_dl_mode Setting the upstream and downstream co-channel 1:
 * Co-channel mode 2: Co-channel mode */
void M5_LoRaWAN::configOTAA(String device_eui, String app_eui, String app_key,
                            String ul_dl_mode) {
    writeCMD("AT+CJOINMODE=0\r\n");
    writeCMD("AT+CDEVEUI=" + device_eui + "\r\n");
    writeCMD("AT+CAPPEUI=" + app_eui + "\r\n");
    writeCMD("AT+CAPPKEY=" + app_key + "\r\n");
    writeCMD("AT+CULDLMODE=" + ul_dl_mode + "\r\n");
}

/*! @brief Configuring ABP access
 *  @param device_addr Node DevAddr
 *  @param app_skey Node AppSKey
 *  @param net_skey Node NwkSKey
 *  @param ul_dl_mode Setting the upstream and downstream co-channel 1:
 * Co-channel mode 2: Co-channel mode */
void M5_LoRaWAN::configABP(String device_addr, String app_skey, String net_skey,
                           String ul_dl_mode) {
    writeCMD("AT+CJOINMODE=1\r\n");
    writeCMD("AT+CDEVADDR=" + device_addr + "\r\n");
    writeCMD("AT+CAPPSKEY=" + app_skey + "\r\n");
    writeCMD("AT+CNWKSKEY=" + net_skey + "\r\n");
    writeCMD("AT+CULDLMODE=" + ul_dl_mode + "\r\n");
}

/*! @brief Set Class mode
 *  @param mode  0: classA 1: classB 2: classC */
void M5_LoRaWAN::setClass(String mode) {
    writeCMD("AT+CCLASS=" + mode + "\r\n");
}

/*! @brief Setting the reception window parameters
 *  @param freq Receiving frequency */
void M5_LoRaWAN::setRxWindow(String freq) {
    writeCMD("AT+CRXP=0,0," + freq + "\r\n");
}

/*! @brief Setting the band mask
 *  @param mask Frequency Point Mask
For channels 0-7, the corresponding mask is 0001, for channels 8-15 it is 0002,
and so on. */
void M5_LoRaWAN::setFreqMask(String mask) {
    writeCMD("AT+CFREQBANDMASK=" + mask + "\r\n");
}

/*! @brief Join the Node */
void M5_LoRaWAN::startJoin() {
    writeCMD("AT+CJOIN=1,0,10,8\r\n");
}

/*! @brief Encode the to hex string the message to be sent
 *  @param str String formatted message to be sent */
String M5_LoRaWAN::encodeMsg(String str) {
    char buf[str.length() + 1];
    char tempbuf[((str.length() + 1) * 2)];
    str.toCharArray(buf, str.length() + 1);
    int i = 0;
    for (const char* p = buf; *p; ++p) {
        sprintf((char*)(tempbuf + i), "%02x", *p);
        i += 2;
    }
    return String(tempbuf);
}

/*! @brief Decode the received hex string to a string
 *  @param hexEncoded received hex string */
String M5_LoRaWAN::decodeMsg(String hexEncoded) {
    if ((hexEncoded.length() & 1) == 0) {
        char buf[hexEncoded.length() + 1];
        char tempbuf[((hexEncoded.length() + 1))];
        hexEncoded.toCharArray(buf, hexEncoded.length() + 1);
        int i = 0;
        for (int loop = 2; loop < hexEncoded.length() + 1; loop += 2) {
            String tmpstr = hexEncoded.substring(loop - 2, loop);
            sprintf(&tempbuf[i], "%c", strtoul(tmpstr.c_str(), nullptr, 16));
            i++;
        }
        return String(tempbuf);
    } else {
        return hexEncoded;
    }
}
