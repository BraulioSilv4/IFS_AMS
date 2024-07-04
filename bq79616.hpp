/*
 *  @file bq79606.h
 *
 *  @author Vince Toledo - Texas Instruments Inc.
 *  @date July 2019
 *  @version 1.0 beta version
 *  @note Built with CCS for Hercules Version: 8.1.0.00011
 *  @note Built for TMS570LS1224 (LAUNCH XL2)
 */

/*****************************************************************************
**
**  Copyright (c) 2011-2017 Texas Instruments
**
******************************************************************************/


#ifndef BQ79616_HPP_
#define BQ79616_HPP_


#include "Arduino.h"
#include "SPI.h"
#include "stdint.h"
#include "stdio.h"
#include "stdbool.h"
#include "string.h"
#include "due_can.h"

//****************************************************
// ***Register defines, choose one of the following***
// ***based on your device silicon revision:       ***
//****************************************************
//#include "A0_reg.h"
#include "B0_reg.h"

// User defines
#define TOTALBOARDS 4     //boards in stack
#define COMM_TIMEOUT 900
#define COMM_FAULT_ID 0x67
#define CELL_TEMP_NUM 8
#define ACTIVECHANNELS 16   //channels to activate (incomplete, does not work right now)~
#define GPIOCHANNELS 8      //channels to read from GPIO
#define BRIDGEDEVICE 0   //
#define MAXcharS (16*2)     //maximum number of chars to be read from the devices (for array creation)
#define BAUDRATE 1000000    //device + uC baudrate
#define nCS 77             //chip select pin
#define SPI_RDY 43           //SPI ready pin
#define nFAULT 46            //fault pin
#define MOSI 75            //SPI MOSI pin
#define SHUTDOWN2 38       //shutdown pin
#define SHUTDOWN1 39       //shutdown pin
#define AIRMinus_STATE 54
#define PRE_STATE 55
#define AIRPlus_STATE 67

// Device defines
#define N_CELLS 16          //number of cells per device
#define CELL_REGISTER_SIZE 2     //size of register in bytes
#define REGISTER_SIZE 1   //size of register in bytes
#define HEADER_SIZE 4       //size of header in bytes
#define FOOTER_SIZE 2       //size of footer in bytes (CRC)
#define RESPONSE_FRAME_SIZE N_CELLS*CELL_REGISTER_SIZE + (HEADER_SIZE + FOOTER_SIZE) //size of frame in bytes
#define GPIO_FRAME_SIZE GPIOCHANNELS*REGISTER_SIZE + (HEADER_SIZE + FOOTER_SIZE) //size of frame in bytes
//#define CELL_TEMP_NUM 2     //number of cell temperature sensors
//#define TEMP_FRAME_SIZE (CELL_TEMP_NUM*REGISTER_SIZE + (HEADER_SIZE + FOOTER_SIZE))*TOTALBOARDS //size of frame in bytes
#define FAULT_FRAME_SIZE (REGISTER_SIZE + (HEADER_SIZE + FOOTER_SIZE))

#define FRMWRT_SGL_R	0x00    //single device READ
#define FRMWRT_SGL_W	0x10    //single device WRITE
#define FRMWRT_STK_R	0x20    //stack READ
#define FRMWRT_STK_W	0x30    //stack WRITE
#define FRMWRT_ALL_R	0x40    //broadcast READ
#define FRMWRT_ALL_W	0x50    //broadcast WRITE
#define FRMWRT_REV_ALL_W 0xE0   //broadcast WRITE reverse direction
                  
// namespace bq{
// Function Prototypes
void SpiWake79600(void);
void SpiAutoAddress();

uint16_t CRC16(char *pBuf, int nLen);

int SpiWriteReg(char bID, uint16_t wAddr, uint64_t dwData, char bLen, char bWriteType);
int SpiWriteFrame(uint16_t bID, uint16_t wAddr, uint16_t * pData, uint16_t bLen, uint8_t bWriteType);
uint32_t SpiCRC16(uint16_t *pBuf, int nLen);
void SpiExchange(uint16_t *data, int len);
int isSPIReady();
void shutdown();

int SpiReadReg(char bID, uint16_t wAddr, uint16_t * pData, char bLen, uint32_t dwTimeOut, char bWriteType);

// void delayms(uint16_t ms);
// void delayus(uint16_t us);

void ResetAllFaults(char bID, char bWriteType);

struct BMS_status{
    uint16_t cell_voltages[16];
    uint16_t cell_temps[8];
    uint8_t status;
    bool fault;
};

extern bool comm_fault;

#endif /* BQ79606_H_ */
//EOF
