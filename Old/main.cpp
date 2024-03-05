/* Include Files */

#include "include/sys_common.h"
#include "include/bq79616.h"
#include "include/bq79600.h"
#include "include/system.h"
#include "include/gio.h"
#include "include/sci.h"
#include "include/spi.h"
#include "include/rti.h"
#include "include/sys_vim.h"

#include "sys_vim.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <string.h>
/* USER CODE END */

/** @fn void main(void)
*   @brief Application main function
*   @note This function is empty by default.
*
*   This function is called after startup.
*   The user can use this function to implement the application.
*/

/* USER CODE BEGIN (2) */
int UART_RX_RDY = 0;
int RTI_TIMEOUT = 0;
/* USER CODE END */

int main(void)
{
/* USER CODE BEGIN (3) */

    //INITIALIZE TMS570LS1224
    systemInit();
    gioInit();
    sciInit();
    spiInit();
    rtiInit();
    sciSetBaudrate(sciREG, 1000000);
    vimInit();
    //_enable_IRQ();

    //write 0xFF during read commands, initialize this 0xFF buffer
    memset(FFBuffer, 0xFF, sizeof(FFBuffer));

    //configure SPI behavior
    dataconfig1_t.CS_HOLD = 1;
    dataconfig1_t.WDEL    =  FALSE;

    //VARIABLES
    uint16 response_frame[(128+6)*TOTALBOARDS]; //store 128 bytes + 6 header bytes for each board

    printf("\n\n\nBeginning Program\n");

    //INITIALIZE 600
    //two wakes in case the MCU had nCS and MOSI = 0 at start (which would put the device in shutdown) or in case the device was previously put in shutdown through a shutdown ping
    SpiWake79600(); //send wake ping to bridge device
    delayus(3500); //wait tSU(WAKE_SHUT), at least 3.5ms
    SpiWake79600(); //send wake ping to bridge device
    delayus(3500); //tSU(WAKE_SHUT), at least 3.5ms

    //INITIALIZE BQ79616-Q1 STACK
    SpiWriteReg(0, CONTROL1, 0x20, 1, FRMWRT_SGL_W); //send wake tone to stack devices
    delayms(11.6*TOTALBOARDS); //wake tone duration is ~1.6ms per board + 10ms per board for each device to wake up from shutdown = 11.6ms per 616 board.

    //AUTO-ADDRESS
    SpiAutoAddress(); //auto address sequence

    //RESET ANY COMM FAULT CONDITIONS FROM STARTUP
    SpiWriteReg(0, FAULT_RST1, 0xFF, 1, FRMWRT_STK_W); //Reset faults on stacked devices
    SpiWriteReg(0, FAULT_RST2, 0xFF, 1, FRMWRT_STK_W); //Reset faults on stacked devices
    SpiWriteReg(0, Bridge_FAULT_RST, 0x22, 1, FRMWRT_SGL_W); //Reset FAULT_COMM and FAULT_SYS on bridge device

    //ENABLE BQ79616-Q1 MAIN ADC
    SpiWriteReg(0, ACTIVE_CELL, 0x0A, 1, FRMWRT_STK_W); //set all cells to active
    SpiWriteReg(0, ADC_CTRL1, 0x06, 1, FRMWRT_STK_W);   //continuous run and MAIN_GO
    delayus(5*TOTALBOARDS + 192);                       //5us reclocking per board and 192us for round robin to complete

    //LOOP VARIABLES
    int channel = 0;            //iterators
    int currentBoard = 0;
    do
    {
        channel = 0;
        currentBoard = 0;
        delayus(192+5*TOTALBOARDS);
        SpiReadReg(0, VCELL16_HI+(16-ACTIVECHANNELS)*2, response_frame, ACTIVECHANNELS*2, 0, FRMWRT_STK_R);

        printf("\n"); //start with a newline to add some extra spacing between each loop
        //only read/print the base device's data if there is no bridge device
        for(currentBoard=0; currentBoard<( BRIDGEDEVICE==1 ? TOTALBOARDS-1 : TOTALBOARDS); currentBoard++)
        {
            printf("BOARD %d:\t",TOTALBOARDS-currentBoard-1);
            //print the data from each active channel (2 bytes each channel)
            for(channel=0; channel<(ACTIVECHANNELS*2); channel+=2)
            {
                int boardByteStart = (ACTIVECHANNELS*2+6)*currentBoard;
                uint16 rawData = (response_frame[boardByteStart+channel+4] << 8) | response_frame[boardByteStart+channel+5];
                float cellVoltage = rawData*0.00019073; //rawData*VLSB_ADC
                printf("%f\t", cellVoltage);
            }
            printf("\n"); //newline per board
        }
    } while(1);

    return 0;
}
