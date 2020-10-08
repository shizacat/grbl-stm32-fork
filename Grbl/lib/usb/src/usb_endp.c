/**
  ******************************************************************************
  * @file    usb_endp.c
  * @author  MCD Application Team
  * @version V4.1.0
  * @date    26-May-2017
  * @brief   Endpoint routines
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_mem.h"
#include "hw_config.h"
#include "usb_istr.h"
#include "usb_pwr.h"

#include "serial.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Interval between sending IN packets in frame number (1 frame = 1ms) */
#define VCOMPORT_IN_FRAME_INTERVAL             5
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t USB_Rx_Buffer[VIRTUAL_COM_PORT_DATA_SIZE];

extern uint8_t serial_tx_buffer[];
extern uint8_t serial_tx_buffer_head;
extern volatile uint8_t serial_tx_buffer_tail;
// extern __IO uint32_t packet_sent;
// extern __IO uint32_t packet_receive;
// extern __IO uint8_t Receive_Buffer[64];
// uint32_t Receive_length;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/*******************************************************************************
* Function Name  : EP1_IN_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/

void EP1_IN_Callback (void)
{
  if ((serial_tx_buffer_head != serial_tx_buffer_tail) && 
      (_GetEPTxStatus(ENDP1) == EP_TX_NAK)
  )
  {
	  uint16_t USB_Tx_length;

    if (serial_tx_buffer_head > serial_tx_buffer_tail) {
      USB_Tx_length = serial_tx_buffer_head - serial_tx_buffer_tail;
    } else {
      USB_Tx_length = TX_BUFFER_SIZE - serial_tx_buffer_tail + serial_tx_buffer_head;
    }

    if (USB_Tx_length != 0) {
      if (USB_Tx_length > 64) {
        USB_Tx_length = 64;
      }

      // UserToPMABufferCopy(&serial_tx_buffer[serial_tx_buffer_tail], ENDP1_TXADDR, USB_Tx_length);
      {
        uint8_t *pbUsrBuf = serial_tx_buffer + serial_tx_buffer_tail;
        uint32_t n = (USB_Tx_length + 1) >> 1;   /* n = (wNBytes + 1) / 2 */
        uint32_t i, temp1;
        uint16_t *pdwVal;
        pdwVal = (uint16_t *)(ENDP1_TXADDR * 2 + PMAAddr);
        for (i = n; i != 0; i--) {
          temp1 = (uint16_t) * pbUsrBuf;
          pbUsrBuf++;
          if (pbUsrBuf - serial_tx_buffer == TX_BUFFER_SIZE){
            pbUsrBuf = serial_tx_buffer;
          }
          *pdwVal++ = temp1 | (uint16_t) * pbUsrBuf << 8;
          pdwVal++;
          pbUsrBuf++;
          if (pbUsrBuf - serial_tx_buffer == TX_BUFFER_SIZE){
            pbUsrBuf = serial_tx_buffer;
          }
        }
      }

	    SetEPTxCount(ENDP1, USB_Tx_length);
	    SetEPTxValid(ENDP1);

      serial_tx_buffer_tail += USB_Tx_length;
      if (serial_tx_buffer_tail >= TX_BUFFER_SIZE){
        serial_tx_buffer_tail -= TX_BUFFER_SIZE;
      }
    }
	}
}

/*******************************************************************************
* Function Name  : EP3_OUT_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP3_OUT_Callback(void)
{
  uint32_t USB_Rx_Cnt = 0;

  /* Get the received data buffer and update the counter */
  USB_Rx_Cnt = GetEPRxCount(ENDP3);
  PMAToUserBufferCopy((unsigned char*)USB_Rx_Buffer, ENDP3_RXADDR, USB_Rx_Cnt);
  
  /* USB data will be immediately processed, this allow next USB traffic being
	NAKed till the end of the USART Xfer */
	OnUsbDataRx(USB_Rx_Buffer, USB_Rx_Cnt);

	/* Enable the receive of data on EP3 */
	SetEPRxValid(ENDP3);
}

/*******************************************************************************
* Function Name  : SOF_Callback
* Description    : Start Of Frame (SOF) callback
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void SOF_Callback(void)
{
	if(bDeviceState == CONFIGURED)
	{
		/* Check the data to be sent through IN pipe */
		EP1_IN_Callback();
	}
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
