/*
 * audio_cb.h
 *
 *  Created on: 25 de mai. de 2026
 *      Author: moreto
 */

#ifndef INC_AUDIO_CB_H_
#define INC_AUDIO_CB_H_


#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>

void audio_init(void);
uint8_t audio_is_streaming(void);

#ifdef __cplusplus
}
#endif

#endif /* INC_AUDIO_CB_H_ */
