/*
 * usb_descriptors.h
 *
 *  Created on: 26 de mai. de 2026
 *      Author: moreto
 */

#ifndef INC_USB_DESCRIPTORS_H_
#define INC_USB_DESCRIPTORS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "tusb.h"

enum
{
  ITF_NUM_AUDIO_CONTROL = 0,
  ITF_NUM_AUDIO_STREAMING,
  ITF_NUM_MIDI,
  ITF_NUM_MIDI_STREAMING,
  ITF_NUM_TOTAL
};

//#define CONFIG_TOTAL_LEN    	(TUD_CONFIG_DESC_LEN + CFG_TUD_AUDIO * TUD_AUDIO20_MIC_ONE_CH_DESC_LEN)
#define CONFIG_TOTAL_LEN    	(TUD_CONFIG_DESC_LEN + TUD_MIDI_DESC_LEN + CFG_TUD_AUDIO * TUD_AUDIO20_LINEIN_TWO_CH_DESC_LEN)


#define EPNUM_AUDIO   0x01
#define EPNUM_MIDI    0x02

// String Descriptor Index
enum {
  STRID_LANGID = 0,
  STRID_MANUFACTURER,
  STRID_PRODUCT,
  STRID_SERIAL,
};



/* Lenght of the descriptor for 2 channels Line IN*/
#define TUD_AUDIO20_LINEIN_TWO_CH_DESC_LEN ( \
  TUD_AUDIO20_DESC_IAD_LEN \
  + TUD_AUDIO20_DESC_STD_AC_LEN \
  + TUD_AUDIO20_DESC_CS_AC_LEN \
  + TUD_AUDIO20_DESC_CLK_SRC_LEN \
  + TUD_AUDIO20_DESC_INPUT_TERM_LEN \
  + TUD_AUDIO20_DESC_OUTPUT_TERM_LEN \
  + TUD_AUDIO20_DESC_FEATURE_UNIT_LEN(2) \
  + TUD_AUDIO20_DESC_STD_AS_LEN \
  + TUD_AUDIO20_DESC_STD_AS_LEN \
  + TUD_AUDIO20_DESC_CS_AS_INT_LEN \
  + TUD_AUDIO20_DESC_TYPE_I_FORMAT_LEN \
  + TUD_AUDIO20_DESC_STD_AS_ISO_EP_LEN \
  + TUD_AUDIO20_DESC_CS_AS_ISO_EP_LEN )

/* UAC2 descriptor for 2 channel Line IN: */
#define TUD_AUDIO20_LINEIN_TWO_CH_DESCRIPTOR(_itfnum, _stridx, _nBytesPerSample, _nBitsUsedPerSample, _epin, _epsize) \
  /* Standard Interface Association Descriptor (IAD) */\
  TUD_AUDIO20_DESC_IAD(_itfnum, 0x02, 0x00),\
  /* Standard AC Interface Descriptor(4.7.1) */\
  TUD_AUDIO20_DESC_STD_AC(_itfnum, 0x00, _stridx),\
  /* Class-Specific AC Interface Header Descriptor(4.7.2) */\
  TUD_AUDIO20_DESC_CS_AC(0x0200, AUDIO20_FUNC_MICROPHONE, TUD_AUDIO20_DESC_CLK_SRC_LEN + TUD_AUDIO20_DESC_INPUT_TERM_LEN + \
		  TUD_AUDIO20_DESC_OUTPUT_TERM_LEN + \
		  TUD_AUDIO20_DESC_FEATURE_UNIT_LEN(2), \
		  AUDIO20_CS_AS_INTERFACE_CTRL_LATENCY_POS),\
  /* Clock Source Descriptor(4.7.2.1) */\
  TUD_AUDIO20_DESC_CLK_SRC(0x04, AUDIO20_CLOCK_SOURCE_ATT_INT_FIX_CLK,\
		  (AUDIO20_CTRL_R << AUDIO20_CLOCK_SOURCE_CTRL_CLK_FRQ_POS), \
		  0x01, 0x00),\
  /* Input Terminal Descriptor(4.7.2.4) */\
  TUD_AUDIO20_DESC_INPUT_TERM(0x01, AUDIO_TERM_TYPE_IN_LINE_CONNECTOR, 0x03, 0x04, 0x02, \
		  AUDIO20_CHANNEL_CONFIG_FRONT_LEFT | AUDIO20_CHANNEL_CONFIG_FRONT_RIGHT, 0x00, \
		  AUDIO20_CTRL_R << AUDIO20_IN_TERM_CTRL_CONNECTOR_POS, 0x00),\
  /* Output Terminal Descriptor(4.7.2.5) */\
  TUD_AUDIO20_DESC_OUTPUT_TERM(0x03,AUDIO_TERM_TYPE_USB_STREAMING, 0x01, 0x02, 0x04, 0x0000, 0x00),\
  /* Feature Unit Descriptor(4.7.2.8) */\
  TUD_AUDIO20_DESC_FEATURE_UNIT(0x02, 0x01, 0x00,\
		  AUDIO20_CTRL_RW << AUDIO20_FEATURE_UNIT_CTRL_MUTE_POS | \
		  AUDIO20_CTRL_RW << AUDIO20_FEATURE_UNIT_CTRL_VOLUME_POS, \
		  AUDIO20_CTRL_RW << AUDIO20_FEATURE_UNIT_CTRL_MUTE_POS | \
		  AUDIO20_CTRL_RW << AUDIO20_FEATURE_UNIT_CTRL_VOLUME_POS, \
		  AUDIO20_CTRL_RW << AUDIO20_FEATURE_UNIT_CTRL_MUTE_POS | \
		  AUDIO20_CTRL_RW << AUDIO20_FEATURE_UNIT_CTRL_VOLUME_POS),\
  /* Standard AS Interface Descriptor(4.9.1) */\
  /* Interface 1, Alternate 0 - default alternate setting with 0 bandwidth */\
  TUD_AUDIO20_DESC_STD_AS_INT((uint8_t)((_itfnum)+1), 0x00, 0x00, 0x00),\
  /* Standard AS Interface Descriptor(4.9.1) */\
  /* Interface 1, Alternate 1 - alternate interface for data streaming */\
  TUD_AUDIO20_DESC_STD_AS_INT((uint8_t)((_itfnum)+1), 0x01, 0x01, 0x00),\
  /* Class-Specific AS Interface Descriptor(4.9.2) */\
  TUD_AUDIO20_DESC_CS_AS_INT(0x03, AUDIO20_CTRL_NONE, AUDIO20_FORMAT_TYPE_I, AUDIO20_DATA_FORMAT_TYPE_I_PCM, \
		  0x02, AUDIO20_CHANNEL_CONFIG_FRONT_LEFT | AUDIO20_CHANNEL_CONFIG_FRONT_RIGHT, 0x00),\
  /* Type I Format Type Descriptor(2.3.1.6 - Audio Formats) */\
  TUD_AUDIO20_DESC_TYPE_I_FORMAT(_nBytesPerSample, _nBitsUsedPerSample),\
  /* Standard AS Isochronous Audio Data Endpoint Descriptor(4.10.1.1) */\
  TUD_AUDIO20_DESC_STD_AS_ISO_EP(_epin, (uint8_t)((uint8_t)TUSB_XFER_ISOCHRONOUS | (uint8_t)TUSB_ISO_EP_ATT_SYNCHRONOUS | \
		  (uint8_t)TUSB_ISO_EP_ATT_DATA), _epsize, 0x01),\
  /* Class-Specific AS Isochronous Audio Data Endpoint Descriptor(4.10.1.2) */\
  TUD_AUDIO20_DESC_CS_AS_ISO_EP(AUDIO20_CS_AS_ISO_DATA_EP_ATT_NON_MAX_PACKETS_OK, AUDIO20_CTRL_NONE, \
		  AUDIO20_CS_AS_ISO_DATA_EP_LOCK_DELAY_UNIT_UNDEFINED, 0x0000)


#ifdef __cplusplus
}
#endif


#endif /* INC_USB_DESCRIPTORS_H_ */
