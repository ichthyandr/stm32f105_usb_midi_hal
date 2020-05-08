/**
  ******************************************************************************
  * @file    usbd_cdc.h
  * @author  MCD Application Team
  * @brief   header file for the usbd_cdc.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                      www.st.com/SLA0044
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_CDC_H
#define __USB_CDC_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

#if 0
/// #define SRAM_BASE             	0x20000000UL /*!< SRAM base address in the alias region */
/// #define SRAM_BB_BASE          	0x22000000UL /*!< SRAM base address in the bit-band region *
#define BITBAND_SRAM(a,b) 			(SRAM_BB_BASE + (a-SRAM_BASE)*32 + (b*4))
#define MAILBOX   					(SRAM_BASE + 1)
#define MBX_B0    					*((volatile uint32_t *)(BITBAND_SRAM(MAILBOX,0))) // Bit 0
#define MBX_B1    					*((volatile uint32_t *)(BITBAND_SRAM(MAILBOX,1))) // Bit 1
#endif

#if 1
#define MIDI_IN_EP                                  0x81  /* EP1 for data IN */
#define MIDI_OUT_EP                                 0x01  /* EP1 for data OUT */
// #define MIDI_DATA_HS_MAX_PACKET_SIZE             512  /* Endpoint IN & OUT Packet size */
#define MIDI_DATA_FS_MAX_PACKET_SIZE                64  /* Endpoint IN & OUT Packet size */
#define MIDI_CMD_PACKET_SIZE                        8  /* Control Endpoint Packet size */

#define USB_MIDI_CONFIG_DESC_SIZ                    101 //133 (2 cables)
#define MIDI_DATA_IN_PACKET_SIZE                    MIDI_DATA_FS_MAX_PACKET_SIZE
#define MIDI_DATA_OUT_PACKET_SIZE                   MIDI_DATA_FS_MAX_PACKET_SIZE
#define APP_RX_DATA_SIZE                            (MIDI_DATA_FS_MAX_PACKET_SIZE * 2) //buffer to send (64*2=128bytes)

#define MIDI_IN_FRAME_INTERVAL		             	1

#define USB_TX_READY            					0       //ready for transfer
#define USB_TX_BUSY             					1       //busy
#define USB_TX_CONTINUE         					2       //ready to continue tx next packet (64 bit)

extern uint8_t flag_USB_DataIn;
uint8_t get_flag_DataIn();
void set_flag_DataIn();
void reset_flag_DataIn();


typedef struct _USBD_MIDI_ItfTypeDef{
  uint16_t (*pIf_MidiRx)    (uint8_t *msg, uint16_t length);
  //uint16_t (*pIf_MidiTx)    (uint8_t *msg, uint16_t length);
} USBD_MIDI_ItfTypeDef;


extern uint8_t APP_Rx_Buffer   [APP_RX_DATA_SIZE];
extern uint32_t APP_Rx_ptr_in;
extern uint32_t APP_Rx_ptr_out;
extern uint32_t APP_Rx_length;
extern uint8_t  USB_Tx_State;

extern uint8_t USB_Rx_Buffer[MIDI_DATA_OUT_PACKET_SIZE];

extern USBD_ClassTypeDef	USBD_MIDI;
#define USBD_MIDI_CLASS    	&USBD_MIDI

uint8_t  USBD_MIDI_RegisterInterface  (USBD_HandleTypeDef   *pdev,
                                      USBD_MIDI_ItfTypeDef *fops);

void USBD_MIDI_SendPacket ();


#else
/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup usbd_cdc
  * @brief This file is the Header file for usbd_cdc.c
  * @{
  */


/** @defgroup usbd_cdc_Exported_Defines
  * @{
  */
#define CDC_IN_EP                                   0x81U  /* EP1 for data IN */
#define CDC_OUT_EP                                  0x01U  /* EP1 for data OUT */
#define CDC_CMD_EP                                  0x82U  /* EP2 for CDC commands */

#ifndef CDC_HS_BINTERVAL
#define CDC_HS_BINTERVAL                          0x10U
#endif /* CDC_HS_BINTERVAL */

#ifndef CDC_FS_BINTERVAL
#define CDC_FS_BINTERVAL                          0x10U
#endif /* CDC_FS_BINTERVAL */

/* CDC Endpoints parameters: you can fine tune these values depending on the needed baudrates and performance. */
#define CDC_DATA_HS_MAX_PACKET_SIZE                 512U  /* Endpoint IN & OUT Packet size */
#define CDC_DATA_FS_MAX_PACKET_SIZE                 64U  /* Endpoint IN & OUT Packet size */
#define CDC_CMD_PACKET_SIZE                         8U  /* Control Endpoint Packet size */

#define USB_CDC_CONFIG_DESC_SIZ                     67U
#define CDC_DATA_HS_IN_PACKET_SIZE                  CDC_DATA_HS_MAX_PACKET_SIZE
#define CDC_DATA_HS_OUT_PACKET_SIZE                 CDC_DATA_HS_MAX_PACKET_SIZE

#define CDC_DATA_FS_IN_PACKET_SIZE                  CDC_DATA_FS_MAX_PACKET_SIZE
#define CDC_DATA_FS_OUT_PACKET_SIZE                 CDC_DATA_FS_MAX_PACKET_SIZE

/*---------------------------------------------------------------------*/
/*  CDC definitions                                                    */
/*---------------------------------------------------------------------*/
#define CDC_SEND_ENCAPSULATED_COMMAND               0x00U
#define CDC_GET_ENCAPSULATED_RESPONSE               0x01U
#define CDC_SET_COMM_FEATURE                        0x02U
#define CDC_GET_COMM_FEATURE                        0x03U
#define CDC_CLEAR_COMM_FEATURE                      0x04U
#define CDC_SET_LINE_CODING                         0x20U
#define CDC_GET_LINE_CODING                         0x21U
#define CDC_SET_CONTROL_LINE_STATE                  0x22U
#define CDC_SEND_BREAK                              0x23U

/**
  * @}
  */


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */

/**
  * @}
  */
typedef struct
{
  uint32_t bitrate;
  uint8_t  format;
  uint8_t  paritytype;
  uint8_t  datatype;
} USBD_CDC_LineCodingTypeDef;

typedef struct _USBD_CDC_Itf
{
  int8_t (* Init)(void);
  int8_t (* DeInit)(void);
  int8_t (* Control)(uint8_t cmd, uint8_t *pbuf, uint16_t length);
  int8_t (* Receive)(uint8_t *Buf, uint32_t *Len);

} USBD_CDC_ItfTypeDef;


typedef struct
{
  uint32_t data[CDC_DATA_HS_MAX_PACKET_SIZE / 4U];      /* Force 32bits alignment */
  uint8_t  CmdOpCode;
  uint8_t  CmdLength;
  uint8_t  *RxBuffer;
  uint8_t  *TxBuffer;
  uint32_t RxLength;
  uint32_t TxLength;

  __IO uint32_t TxState;
  __IO uint32_t RxState;
}
USBD_CDC_HandleTypeDef;



/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */

extern USBD_ClassTypeDef  USBD_CDC;
#define USBD_CDC_CLASS    &USBD_CDC
/**
  * @}
  */

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
uint8_t  USBD_CDC_RegisterInterface(USBD_HandleTypeDef   *pdev,
                                    USBD_CDC_ItfTypeDef *fops);

uint8_t  USBD_CDC_SetTxBuffer(USBD_HandleTypeDef   *pdev,
                              uint8_t  *pbuff,
                              uint16_t length);

uint8_t  USBD_CDC_SetRxBuffer(USBD_HandleTypeDef   *pdev,
                              uint8_t  *pbuff);

uint8_t  USBD_CDC_ReceivePacket(USBD_HandleTypeDef *pdev);

uint8_t  USBD_CDC_TransmitPacket(USBD_HandleTypeDef *pdev);
/**
  * @}
  */
#endif

#ifdef __cplusplus
}
#endif

#endif  /* __USB_CDC_H */
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
