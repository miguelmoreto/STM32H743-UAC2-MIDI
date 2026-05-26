/*
 * midi.h
 *
 *  Created on: 25 de mai. de 2026
 *      Author: moreto
 */

#ifndef INC_MIDI_H_
#define INC_MIDI_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Control Change definitions: */
#define MIDI_CC_TEST 0x14

/* MIDI messages type definitions */
typedef enum {
	NOTE_OFF = 0x08,	// Note Off
	NOTE_ON = 0x09,		// Note On
	CC = 0x0B,			// Control Change
	PC = 0x0C,			// Program Change
	PB = 0x0E			// Pitch Bend
} MIDI_MsgTypeEnum_typedef;


/* This struct takes only 4 bytes: */
typedef struct {
	uint8_t CIN : 4;					// Code Index Number (4 LSB bits of byte 0).
	uint8_t cable : 4;					// Cable Number (4 MSB bits of byte 0)
	uint8_t channel : 4;				// Channel number (4 LSB bits of byte 1).
	MIDI_MsgTypeEnum_typedef type : 4;	// Message type (4 MSB bits of byte 1). This is the same as CIN
	uint8_t data1;						// Byte 2
	//uint8_t : 0; 			// Force a new byte as data 1 is only 7 bits.
	uint8_t data2;  					// Byte 3
} MIDI_MsgStruct_typedef;

/* With the union type we can have the struct and the byte array in the same
 * variable (no need to declare temporary arrays when sending or receiving data). */
typedef union {
	MIDI_MsgStruct_typedef fields;
	uint8_t array[4];
} MIDI_MsgUnion_typedef;


#ifdef __cplusplus
}
#endif

#endif /* INC_MIDI_H_ */
