/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v2.0_Cube
  * @brief          : Usb device for Virtual Com Port.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
#define NEXTBYTE(idx, mask) (mask & (idx + 1))


tUsbMidiCable usbmidicable1 = { .curidx=0, .rdidx=0, .buf={0,} };
//tUsbMidiCable usbmidicable2;

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;
extern USBD_HandleTypeDef hUsbDeviceFS;

// basic midi rx/tx functions
static uint16_t MIDI_DataRx(uint8_t *msg, uint16_t length);
static uint16_t MIDI_DataTx(uint8_t *msg, uint16_t length);

USBD_MIDI_ItfTypeDef USBD_Interface_fops_FS =
{
  MIDI_DataRx//,
  //MIDI_DataTx
};

static uint8_t g_SysEx = 0;
void USBD_AddEvent( uint8_t cable ){

	uint8_t 		txbuf[4]= {0,};
	uint8_t			byte	= 0;
	tUsbMidiCable* 	pcable 	= NULL;

	if (hUsbDeviceFS.dev_state != USBD_STATE_CONFIGURED) return;
	if ( 0 == cable ) pcable = &usbmidicable1;
	if ( NULL == pcable ) return;
	if ( pcable->curidx == pcable->rdidx ){ return;}
	byte = pcable->buf[ pcable->rdidx ];					// get byte
	pcable->rdidx = NEXTBYTE(pcable->rdidx, USBMIDIMASK);	// move read index
	cable <<= 4;

	if ( ((byte >> 4) & 0x0f) == 0x08 ){ // note off
		txbuf[0] = cable + 0x08;
		txbuf[1] = byte;
		txbuf[2] = pcable->buf[ pcable->rdidx ];
		pcable->rdidx = NEXTBYTE(pcable->rdidx, USBMIDIMASK);
		txbuf[3] = pcable->buf[ pcable->rdidx ];
		pcable->rdidx = NEXTBYTE(pcable->rdidx, USBMIDIMASK);
	}
	else if (((byte >> 4) & 0x0f) == 0x09 ){ // note on
		txbuf[0] = cable + 0x09;
		txbuf[1] = byte;
		txbuf[2] = pcable->buf[ pcable->rdidx ];
		pcable->rdidx = NEXTBYTE(pcable->rdidx, USBMIDIMASK);
		txbuf[3] = pcable->buf[ pcable->rdidx ];
		pcable->rdidx = NEXTBYTE(pcable->rdidx, USBMIDIMASK);
	}
	else if (((byte >> 4) & 0x0f) == 0x0A){ //Polyphonic Key Pressure
		txbuf[0] = cable + 0x0A;
		txbuf[1] = byte;
		txbuf[2] = pcable->buf[ pcable->rdidx ];
		pcable->rdidx = NEXTBYTE(pcable->rdidx, USBMIDIMASK);
		txbuf[3] = pcable->buf[ pcable->rdidx ];
		pcable->rdidx = NEXTBYTE(pcable->rdidx, USBMIDIMASK);
	}
	else if (((byte >> 4) & 0x0f) == 0x0B){ //Control Change
		txbuf[0] = cable + 0x0B;
		txbuf[1] = byte;
		txbuf[2] = pcable->buf[ pcable->rdidx ];
		pcable->rdidx = NEXTBYTE(pcable->rdidx, USBMIDIMASK);
		txbuf[3] = pcable->buf[ pcable->rdidx ];
		pcable->rdidx = NEXTBYTE(pcable->rdidx, USBMIDIMASK);
	}
	else if (((byte >> 4) & 0x0f) == 0x0C){ //Program Change
		txbuf[0] = cable + 0x0C;
		txbuf[1] = byte;
		txbuf[2] = pcable->buf[ pcable->rdidx ];
		pcable->rdidx = NEXTBYTE(pcable->rdidx, USBMIDIMASK);
	}
	else if (((byte >> 4) & 0x0f) == 0x0D){ //Channel Pressure
		txbuf[0] = cable + 0x0D;
		txbuf[1] = byte;
		txbuf[2] = pcable->buf[ pcable->rdidx ];
		pcable->rdidx = NEXTBYTE(pcable->rdidx, USBMIDIMASK);
	}
	else if (((byte >> 4) & 0x0f) == 0x0E){ //Pitch Bend Change
		txbuf[0] = cable + 0x0E;
		txbuf[1] = byte;
		txbuf[2] = pcable->buf[ pcable->rdidx ];
		pcable->rdidx = NEXTBYTE(pcable->rdidx, USBMIDIMASK);
		txbuf[3] = pcable->buf[ pcable->rdidx ];
		pcable->rdidx = NEXTBYTE(pcable->rdidx, USBMIDIMASK);
	}
	else if ((byte == 0xF1)||(byte == 0xF3)){ //MIDI Time Code Quarter Frame || Song Select.
		txbuf[0] = cable + 0x02;
		txbuf[1] = byte;
		txbuf[2] = pcable->buf[ pcable->rdidx ];
		pcable->rdidx = NEXTBYTE(pcable->rdidx, USBMIDIMASK);
	}
	else if ((byte == 0xF6) // Tune Request
			||(byte>= 0xF8)){// single byte messages
		txbuf[0] = cable + 0x05;
		txbuf[1] = byte;
	}
	else if (byte == 0xF2){ //Song Position Pointer
		txbuf[0] = cable + 0x03;
		txbuf[1] = byte;
		txbuf[2] = pcable->buf[ pcable->rdidx ];
		pcable->rdidx = NEXTBYTE(pcable->rdidx, USBMIDIMASK);
		txbuf[3] = pcable->buf[ pcable->rdidx ];
		pcable->rdidx = NEXTBYTE(pcable->rdidx, USBMIDIMASK);
	}
	else if((0==g_SysEx)&&(byte=0xF0) ){// start SysEx
		txbuf[0] = cable + 0x04;
		txbuf[1] = byte;
		txbuf[2] = pcable->buf[ pcable->rdidx ];
		pcable->rdidx = NEXTBYTE(pcable->rdidx, USBMIDIMASK);
		txbuf[3] = pcable->buf[ pcable->rdidx ];
		pcable->rdidx = NEXTBYTE(pcable->rdidx, USBMIDIMASK);
		g_SysEx = 1;
	}
	else if (1==g_SysEx){
		for(uint8_t i=1; i<=3; i++ ){
			txbuf[i] = byte;
			if (0xF7==byte){
				switch (i){
				case 1:
					txbuf[0] = cable + 0x05;
					break;
				case 2:
					txbuf[0] = cable + 0x06;
					break;
				case 3:
					txbuf[0] = cable + 0x07;
					break;
				}
				g_SysEx = 0;
				break;
			}
			if ( i<3 ){
				byte = pcable->buf[ pcable->rdidx ];
				pcable->rdidx = NEXTBYTE(pcable->rdidx, USBMIDIMASK);
			}
			if ((3==i)&&(1==g_SysEx)) txbuf[0] = cable + 0x04;
		}
	}
	else return;

	MIDI_DataTx(txbuf, 4);
}

//Start transfer
void USBD_SendMidiMessages(void)
{
  if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED)
  {
    if (!USB_Tx_State)
      USBD_MIDI_SendPacket();
    else
      USB_Tx_State = USB_TX_CONTINUE;
  }
  else
  {
      USB_Tx_State = USB_TX_READY;
  }
}


/*void OTG_FS_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}*/

/*
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}
*/


//fill midi tx buffer
static uint16_t MIDI_DataTx(uint8_t *msg, uint16_t length)
{
  uint16_t i = 0;
  while (i < length) {
    APP_Rx_Buffer[APP_Rx_ptr_in] = *(msg + i);
    APP_Rx_ptr_in++;
    i++;
    if (APP_Rx_ptr_in == APP_RX_DATA_SIZE) {
      APP_Rx_ptr_in = 0;
    }
  }
  return USBD_OK;
}


//process recived midi data
static uint16_t MIDI_DataRx(uint8_t* msg, uint16_t length)
{
  uint16_t cnt;
  tUsbMidiCable* pcable;

  for (cnt = 0; cnt < length; cnt += 4)
  {
    switch ( msg[cnt] >> 4 ) {
    case 0:
      pcable = &usbmidicable1;
      break;
    /*case 1:
      pcable = &usbmidicable2;
      break;*/
    default:
      continue;
    };

    switch ( msg[cnt] & 0x0F ) {
    case 0x0:
    case 0x1:
      continue;
    case 0x5:
    case 0xF:
      pcable->buf[ pcable->curidx ] = msg[ cnt+1 ];
      pcable->curidx = NEXTBYTE(pcable->curidx, USBMIDIMASK);
      break;
    case 0x2:
    case 0x6:
    case 0xC:
    case 0xD:
      pcable->buf[ pcable->curidx ] = msg[ cnt+1 ];
      pcable->curidx = NEXTBYTE(pcable->curidx, USBMIDIMASK);
      pcable->buf[ pcable->curidx ] = msg[ cnt+2 ];
      pcable->curidx = NEXTBYTE(pcable->curidx, USBMIDIMASK);
      break;
    default:
      pcable->buf[ pcable->curidx ] = msg[ cnt+1 ];
      pcable->curidx = NEXTBYTE(pcable->curidx, USBMIDIMASK);
      pcable->buf[ pcable->curidx ] = msg[ cnt+2 ];
      pcable->curidx = NEXTBYTE(pcable->curidx, USBMIDIMASK);
      pcable->buf[ pcable->curidx ] = msg[ cnt+3 ];
      pcable->curidx = NEXTBYTE(pcable->curidx, USBMIDIMASK);
      break;
    };
  };

  return 0;
}



/* USER CODE END PV */
#if 0
/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */
/* Define size for the receive and transmit buffer over CDC */
/* It's up to user to redefine and/or remove those define */
#define APP_RX_DATA_SIZE  1000
#define APP_TX_DATA_SIZE  1000
/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t* pbuf, uint32_t *Len);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS =
{
  CDC_Init_FS,
  CDC_DeInit_FS,
  CDC_Control_FS,
  CDC_Receive_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the CDC media low layer over the FS USB IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_FS(void)
{
  /* USER CODE BEGIN 3 */
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_FS(void)
{
  /* USER CODE BEGIN 4 */
  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 5 */
  switch(cmd)
  {
    case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

    case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

    case CDC_SET_COMM_FEATURE:

    break;

    case CDC_GET_COMM_FEATURE:

    break;

    case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
    case CDC_SET_LINE_CODING:

    break;

    case CDC_GET_LINE_CODING:

    break;

    case CDC_SET_CONTROL_LINE_STATE:

    break;

    case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will block any OUT packet reception on USB endpoint
  *         untill exiting this function. If you exit this function before transfer
  *         is complete on CDC interface (ie. using DMA controller) it will result
  *         in receiving more data while previous ones are still not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Receive_FS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 6 */
  USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUsbDeviceFS);
  return (USBD_OK);
  /* USER CODE END 6 */
}

/**
  * @brief  CDC_Transmit_FS
  *         Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  *         @note
  *
  *
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 7 */
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceFS.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
  /* USER CODE END 7 */
  return result;
}
#endif
/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
