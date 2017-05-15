#include "rn1810.h"
#include "mcc_generated_files/mcc.h"

#define FCY 8000000UL

#include <libpic30.h>
#include <string.h>
#include <stdio.h>

RN1810_STRUCT rn1810;
const uint8_t * WIFLY_CMD = "$$$";
const uint8_t * WIFLY_APPMODE = "apmode\r\n";

void parseScanResponse() {
    int bytesInBuffer = 0;
    uint8_t buffer[255];
    if (UART1_TRANSFER_STATUS_RX_DATA_PRESENT & UART1_TransferStatusGet()) {
        __delay_ms(100);
        bytesInBuffer = 256 - UART1_ReceiveBufferSizeGet();
        UART1_ReadBuffer(buffer, bytesInBuffer);
        return;
    }
}

bool RN1810_Read(uint8_t * buffer) {
    int bytesInBuffer = 0;
    if (UART1_TRANSFER_STATUS_RX_DATA_PRESENT & UART1_TransferStatusGet()) {
        __delay_ms(100);
        bytesInBuffer = 256 - UART1_ReceiveBufferSizeGet();
        UART1_ReadBuffer(buffer, bytesInBuffer);

        return true;
    }
    return false;
}

void RN1810_ServerTask() {
    uint8_t buffer[255] = "";
    uint8_t response[32] = "";
    unsigned char* token;
    unsigned char* cmd;
    unsigned char* value;
    static bool open;

    if (RN1810_Read(buffer)) {
        if (hasExp("*OPEN*", buffer)) {
            LED_2_SetHigh();
            open = true;
        }
        if (hasExp("*CLOS*", buffer)){
            LED_2_SetLow();    
            open = false;
        }
    }if(open){
     sprintf(response, "OK \n~");
            UART1_WriteBuffer(response, strlen(response));
            __delay_ms(20);
    }
}

void RN1810_DispatchCommand(uint8_t *cmd, uint8_t *value) {
    uint8_t response[128] = "";
    if (strncmp(cmd, "ssid", 4) == 0) {
        sprintf(rn1810.apSSID, "%s", value);
        sprintf(response, "OK %s %s\n~", cmd, value);
        UART1_WriteBuffer(response, strlen(response));
        return;
    }
    if (strncmp(cmd, "sec", 3) == 0) {
        rn1810.securityMode = atoi(value);
        sprintf(response, "OK %s %s\n~", cmd, value);
        UART1_WriteBuffer(response, strlen(response));
        return;
    }
    if (strncmp(cmd, "pass", 4) == 0) {
        sprintf(rn1810.passphrase, "%s", value);
        sprintf(response, "OK %s %s\n~", cmd, value);
        UART1_WriteBuffer(response, strlen(response));
        return;
    }
    if (strncmp(cmd, "start$wlan", 10) == 0) {
        sprintf(response, "OK %s\n~", cmd);
        UART1_WriteBuffer(response, strlen(response));

        if (!RN1810_JoinNetwork(rn1810.apSSID, rn1810.securityMode, rn1810.passphrase, "192.168.1.104", 9999)) {
            RN1810_ExitCMD();
        }
        return;


    }

    sprintf(response, "ERR\n~");
    UART1_WriteBuffer(response, strlen(response));
}

bool hasExp(char* exp, char* str) {
    uint8_t buff[256];
    char* line;
    char* strPtr = str;
    int expectedSize = strlen(exp);
    int strSize = strlen(str);
    int pos = 0;
    while (pos < strSize) {
        buff[pos] = *strPtr;
        pos++;
        strPtr++;
    }
    line = strtok(buff, "\r\n");
    while (line != NULL) {
        if (strncmp(line, exp, expectedSize) == 0)
            return true;
        line = strtok(NULL, "\r\n");
    }
    return false;
}

bool validResponse(char* expected) {
    int bytesInBuffer = 0;
    uint8_t receiveBuffer[256];
    char* line;
    int expectedSize = strlen(expected);
    if (UART1_TRANSFER_STATUS_RX_DATA_PRESENT & UART1_TransferStatusGet()) {
        bytesInBuffer = 256 - UART1_ReceiveBufferSizeGet();
        UART1_ReadBuffer(receiveBuffer, bytesInBuffer);
        line = strtok(receiveBuffer, "\r\n");
        while (line != NULL) {
            if (strncmp(line, expected, expectedSize) == 0)
                return true;
            line = strtok(NULL, "\r\n");
        }

    }
    return false;
}

bool isValidReadyMessage(char* buff) {
    char* line = strtok(buff, "\r\n");
    while (line != NULL) {
        if (strncmp(line, "*READY*", 7) == 0)
            return true;
        line = strtok(NULL, "\r\n");
    }
    return false;
}

bool findExp(char* exp, uint16_t timeout) {
    int bytesInBuffer = 0;
    uint8_t buffer[256];
    uint16_t start = TMR1_SoftwareCounterGet();
    uint16_t bufferPosition = 0;
    uint8_t chunks = 0;
    while (TMR1_ElapsedMilliseconds(start) < timeout) {
        if (UART1_TRANSFER_STATUS_RX_DATA_PRESENT & UART1_TransferStatusGet()) {
            bytesInBuffer = 256 - UART1_ReceiveBufferSizeGet() - bufferPosition;
            if (bytesInBuffer > 0)
                chunks++;
            while (bytesInBuffer > 0) {
                buffer[bufferPosition] = UART1_Peek(bufferPosition);
                bytesInBuffer--;
                bufferPosition++;
                start = TMR1_SoftwareCounterGet();
            }
            if (buffer[0] == '\0')
                buffer[0] = '.';
            if (hasExp(exp, buffer)) {
                return true;
            }
        }
    }
    return false;
}

void RN1810_Execute(char * command, uint8_t * response) {
    int bytesInBuffer = 0;
    UART1_WriteBuffer(command, strlen(command));
    __delay_ms(200);
    if (UART1_TRANSFER_STATUS_RX_DATA_PRESENT & UART1_TransferStatusGet()) {
        bytesInBuffer = 256 - UART1_ReceiveBufferSizeGet();
        UART1_ReadBuffer(response, bytesInBuffer);
    } else {
        sprintf(response, "\0");
    }

}

bool RN1810_ExitCMD() {
    UART1_WriteBuffer("exit\r\n", 6);
    __delay_ms(300);
    if (!validResponse("EXIT")) {
        return false;
    }
    rn1810.comunicationMode = 0;
    return true;
}

bool RN1810_InitCMD() {
    /*
    if (rn1810.comunicationMode)
        return true;
     */
    __delay_ms(250);
    UART1_WriteBuffer(WIFLY_CMD, 3);
    __delay_ms(250);
    if (!findExp("CMD", 1000)) {
        UART1_ResetRXBuffer();
        return false;
    }
    UART1_ResetRXBuffer();
    return true;
}

void RN1810_Init() {
    rn1810.conectionOpen = false;
    RN_1810_RESET_SetLow();
    __delay_ms(100);
    RN_1810_RESET_SetHigh();
    UART1_Initialize();
    __delay_ms(100);
    if (findExp("*READY*", 10000)) {
        rn1810.operationState = READY;
        LED_2_SetHigh();
    } else {
        rn1810.operationState = NOT_READY;
    }
    UART1_ResetRXBuffer();
}

bool RN1810_Save() {
    UART1_WriteBuffer("save\r\n", 6);
    __delay_ms(200);
    if (!validResponse("Verify config data: succeeded")) {
        return false;
    }
    return true;
}

bool RN1810_Open(char* host, uint16_t port) {
    if (!RN1810_InitCMD())
        return false;
    char command[128];
    int commandSize = 0;
    sprintf(command, "open %s %d\r\n", host, port);
    commandSize = strlen(command);
    UART1_WriteBuffer(command, commandSize);
    __delay_ms(200);

    return true;
}

bool RN1810_Reboot() {
    UART1_WriteBuffer("reboot\r\n", 8);
    if (!findExp("*READY*", 1000)) {
        UART1_ResetRXBuffer();
        return false;
    }
    UART1_ResetRXBuffer();
    return true;
}

bool RN1810_JoinNetwork(char* ssid, uint8_t sec, char* pass, char* host, uint16_t port) {
    if (!RN1810_InitCMD())
        return false;
    char command[64];
    int commandSize = 0;
    UART1_WriteBuffer("set ip dhcp 1\r\n", 15);
    __delay_ms(100);
    if (!validResponse("AOK")) {
        return false;
    }

    UART1_WriteBuffer("set dhcp hostname eMeterWF-XFGH6754\r\n", 37);
    __delay_ms(100);
    if (!validResponse("AOK")) {
        return false;
    }

    sprintf(command, "set wlan ssid %s\r\n", ssid);
    commandSize = strlen(command);
    UART1_WriteBuffer(command, commandSize);
    __delay_ms(100);
    if (!validResponse("AOK")) {
        return false;
    }
    if (sec > 0) {
        sprintf(command, "set wlan phrase %s\r\n", pass);
        commandSize = strlen(command);
        UART1_WriteBuffer(command, commandSize);
        __delay_ms(100);
        if (!validResponse("AOK")) {
            return false;
        }
        sprintf(command, "set wlan auth 4\r\n");
        commandSize = strlen(command);
        UART1_WriteBuffer(command, commandSize);
        __delay_ms(100);
        if (!validResponse("AOK")) {
            return false;
        }
    }
    UART1_WriteBuffer("set wlan join 1\r\n", 17);
    __delay_ms(100);
    if (!validResponse("AOK")) {
        return false;
    }

    UART1_WriteBuffer("set sys autoconn 2\r\n", 20);
    __delay_ms(100);
    if (!validResponse("AOK")) {
        return false;
    }

    UART1_WriteBuffer("set ip protocol 0x08\r\n", 22);
    __delay_ms(100);
    if (!validResponse("AOK")) {
        return false;
    }


    sprintf(command, "set ip host %s \r\n", host);
    commandSize = strlen(command);
    UART1_WriteBuffer(command, commandSize);
    __delay_ms(100);
    if (!validResponse("AOK")) {
        return false;
    }
    sprintf(command, "set ip remote %d\r\n", port);
    commandSize = strlen(command);
    UART1_WriteBuffer(command, commandSize);
    __delay_ms(100);
    if (!validResponse("AOK")) {
        return false;
    }

    RN1810_Save();
    RN1810_Reboot();
    return true;
}

void RN1810_Scan() {
    if (RN1810_InitCMD()) {
        UART1_WriteBuffer("scan\r\n", 6);
        __delay_ms(1000);
        parseScanResponse();
        RN1810_ExitCMD();
    }
}

bool RN1810_StartAP(char* ssid, char* pass) {

    char command[64];
    int commandSize = 0;

    UART1_WriteBuffer("set comm idle 5\r\n", 17);
    __delay_ms(100);
    if (!validResponse("AOK")) {
        return false;
    }
    UART1_WriteBuffer("set comm match 0x7e 0\r\n", 23);
    __delay_ms(100);
    if (!validResponse("AOK")) {
        return false;
    }
    UART1_WriteBuffer("set wlan join 7\r\n", 17);
    __delay_ms(100);
    if (!validResponse("AOK")) {
        return false;
    }
    UART1_WriteBuffer("set apmode channel 6\r\n", 22);
    __delay_ms(100);
    if (!validResponse("AOK")) {
        return false;
    }
    sprintf(command, "set apmode ssid %s\r\n", ssid);
    commandSize = strlen(command);
    UART1_WriteBuffer(command, commandSize);
    __delay_ms(100);
    if (!validResponse("AOK")) {
        return false;
    }
    sprintf(command, "set apmode passphrase %s\r\n", pass);
    commandSize = strlen(command);
    UART1_WriteBuffer(command, commandSize);
    __delay_ms(100);
    if (!validResponse("AOK")) {
        return false;
    }
    UART1_WriteBuffer("set ip protocol 0x02\r\n", 22);
    __delay_ms(100);
    if (!validResponse("AOK")) {
        return false;
    }

    return true;
}

void RN1810_Task() {
    switch (rn1810.operationState) {
        case READY:

            break;
    }
}




