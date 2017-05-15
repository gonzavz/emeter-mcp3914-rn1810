/* 
 * File:   rn1810.h
 * Author: gonza
 *
 * Created on February 22, 2017, 11:06 AM
 */

#ifndef RN1810_H
#define	RN1810_H
#include "GenericTypeDefs.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#ifdef	__cplusplus
extern "C" {
#endif
    
    typedef enum {
        NOT_READY = 0,
        REBOOTING,
        READY
    } RN1810_OPERATION_STATE;
    
    typedef struct __attribute__((__packed__)) {
        char apSSID[32];
        uint8_t securityMode;
        char passphrase[128];
        RN1810_OPERATION_STATE operationState; // INITIALIZING, READY
        uint8_t wirelessMode;
        uint8_t comunicationMode; // 0 = data mode, 1 = command mode
        bool conectionOpen;
        char host[128];
        uint16_t hostPort;
    } RN1810_STRUCT;

    bool RN1810_StartAP(char* ssid, char* pass);
    bool RN1810_InitCMD();
    bool RN1810_ExitCMD();
    bool RN1810_Reboot();
    bool RN1810_Save();
    void RN1810_Execute(char * command, uint8_t* response);
    void RN1810_HardReboot();
    void RN1810_Task();
    void RN1810_Init();
    bool RN1810_ExitCMD();
    void RN1810_ServerTask();
    void RN1810_Scan();
    bool RN1810_Open(char* host, uint16_t port);
    bool RN1810_JoinNetwork(char* ssid, uint8_t sec, char* pass, char* host, uint16_t port);
    bool RN1810_Read(uint8_t * buffer);
    bool hasExp(char* exp, char* str);
#ifdef	__cplusplus
}
#endif

#endif	/* RN1810_H */

