/**
 * This is free and unencumbered software released into the public domain.
 * 
 * Anyone is free to copy, modify, publish, use, compile, sell, or
 * distribute this software, either in source code form or as a compiled
 * binary, for any purpose, commercial or non-commercial, and by any
 * means.
 * 
 * In jurisdictions that recognize copyright laws, the author or authors
 * of this software dedicate any and all copyright interest in the
 * software to the public domain. We make this dedication for the benefit
 * of the public at large and to the detriment of our heirs and
 * successors. We intend this dedication to be an overt act of
 * relinquishment in perpetuity of all present and future rights to this
 * software under copyright law.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 * 
 * For more information, please refer to <http://unlicense.org>
 *
 * WWW: https://github.com/cjwagenius/crc8
 *
 **/

#ifndef CRC8_H
#define CRC8_H

#include <stdint.h>

#define CRC8_REFIO_TRUE  0x01
#define CRC8_REFIO_FALSE 0x80

typedef struct crc8 {
	uint8_t crc;		/* init & result  */
	uint8_t poly;		/* polynominal    */
	uint8_t refio;		/* refin & refout */
	uint8_t xor;		/* xor out        */
} crc8_t;

/** predefined crc8-recipes
 *
 * to init a static crc8-struct:
 *
 * 	struct crc8 crc = CRC8_INIT(CRC8_SMBUS);
 *
 *
 * observe that the polynominal of the recipes below that are using refin &
 * refout are bit-reversed. read crc8_init()-documentation for more info.
 *
 **/
#define CRC8_INIT(t) (struct crc8)t;
/*	   name		  init  poly   refin & refout   xor    */
/*------------------------------------------------------------ */
#define CRC8_AUTOSAR	{ 0xff, 0x2f, CRC8_REFIO_FALSE, 0xff }
#define CRC8_BLUETOOTH	{ 0x00, 0xe5, CRC8_REFIO_TRUE,  0x00 }
#define CRC8_CDMA2000	{ 0xff, 0x9b, CRC8_REFIO_FALSE, 0x00 }
#define CRC8_DARC	{ 0x00, 0x9c, CRC8_REFIO_TRUE,  0x00 }
#define CRC8_DVBS2	{ 0x00, 0xd5, CRC8_REFIO_FALSE, 0x00 }
#define CRC8_EBU	{ 0xff, 0xb8, CRC8_REFIO_TRUE,  0x00 }
#define CRC8_GSMA	{ 0x00, 0x1d, CRC8_REFIO_FALSE, 0x00 }
#define CRC8_GSMB	{ 0x00, 0x49, CRC8_REFIO_FALSE, 0xff }
#define CRC8_ICODE	{ 0xfd, 0x1d, CRC8_REFIO_FALSE, 0x00 }
#define CRC8_ITUI	{ 0x00, 0x07, CRC8_REFIO_FALSE, 0x55 } /* aka i-432-1 */
#define CRC8_LTE	{ 0x00, 0x9b, CRC8_REFIO_FALSE, 0x00 }
#define CRC8_MAXIM	{ 0x00, 0x8c, CRC8_REFIO_TRUE,  0x00 }
#define CRC8_MIFAREMAD	{ 0xc7, 0x1d, CRC8_REFIO_FALSE, 0x00 }
#define CRC8_NRSC5	{ 0xff, 0x31, CRC8_REFIO_FALSE, 0x00 }
#define CRC8_OPENSAFETY	{ 0x00, 0x2f, CRC8_REFIO_FALSE, 0x00 }
#define CRC8_ROHC	{ 0xff, 0xe0, CRC8_REFIO_TRUE,  0x00 }
#define CRC8_SAEJ1859	{ 0xff, 0x1d, CRC8_REFIO_FALSE, 0xff }
#define CRC8_SMBUS	{ 0x00, 0x07, CRC8_REFIO_FALSE, 0x00 } /* aka crc8 */
#define CRC8_TECH3250	{ 0xff, 0xb8, CRC8_REFIO_TRUE,  0x00 }
#define CRC8_WCDMA	{ 0x00, 0xd9, CRC8_REFIO_TRUE,  0x00 }

extern uint8_t crc8(struct crc8 *crc, const void *data, size_t len);
extern uint8_t crc8_fin(struct crc8 *crc);
extern void crc8_init(struct crc8 *crc);
extern uint8_t crc8_upd(struct crc8 *crc, uint8_t byte);

#ifdef CRC8_IMPL

uint8_t crc8(struct crc8 *crc, const void *data, size_t len)
{
	/** process multiple bytes.
	 *
	 * a convenience function to process multiple bytes at once. this
	 * function doesn't finish the crc-process off, so it may be run
	 * multiple times. if your recipe needs to xor the result, you'll
	 * have to finish the crc-process with crc8_fin().
	 *
	 * returns the resulting crc-value.
	 **/

	const uint8_t *b;

	for (b = data; len; len --) {
		crc8_upd(crc, *b);
		b ++;
	}

	return crc->crc;
}
uint8_t crc8_fin(struct crc8 *crc)
{
	/** finishing a crc-process off
	 *
	 * you may (but doesn't have to) skip calling this function if your
	 * crc-recipe doesn't xor the result.
	 *
	 * returns the finalized crc-value.
	 *
	 **/

	crc->crc ^= crc->xor;

	return crc->crc;
}
void crc8_init(struct crc8 *crc)
{
	/** initialize a custom crc8 recipe.
	 *
	 * all this function does, is to bit-reverse the polynominal if
	 * refin & refout are used by the recipe. so if your custom recipe
	 * doesn't use refin & refout, or if you bit-reverse the polynominal
	 * yourself, you can skip calling this function.
	 *
	 * example:
	 *
	 * 	struct crc8 crc;
	 *
	 * 	crc.crc = 0x00;			// init-value
	 *	crc.poly = 0x31;		// polynominal
	 *	crc.refio = CRC8_REFIO_FALSE;	// whether to refin & refout
	 *	crc.xor = 0x00;			// what to xor the result with
	 *
	 *	crc8_init(&crc);
	 *
	 **/

	if (crc->refio & 0x01) {
		uint8_t i, b = 0;
		for (i = 0; i < 8; i ++) {
			b <<= 1;
			b |= crc->poly & 0x01;
			crc->poly >>= 1;
		}
		crc->poly = b;
	} else {
		crc->refio = 0x80;
	}
}
uint8_t crc8_upd(struct crc8 *crc, uint8_t byte)
{
	/** updates the crc8 with one byte.
	 *
	 * returns the current crc-value.
	 *
	 **/

	uint8_t c;
	uint8_t i;

	c = crc->crc ^ byte;
	
	for (i = 0; i < 8; i ++) {
		uint8_t p = c & crc->refio;
		c = (crc->refio & 0x80) ? c << 1 : c >> 1;
		c ^= p ? crc->poly : 0x00;
	}

	return (crc->crc = c);
}

#endif /* CRC8_IMPL */
#endif /* CRC8_H */

