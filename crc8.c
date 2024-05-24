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
 * ----------------------------------------------------------------------------
 *
 * calculates multiple crc-values of a string of data represented in hex. this
 * is only an example on how to use crc8.h and isn't a part of the library.
 *
 * usage: crc8_list [+]hexadecimal-string [recipe]
 *
 * 	+	the list will be ordered incremental by crc-value. otherwise
 * 		it's ordered in alphabetical order.
 *
 * 	recipe	a custom crc8-recipe in the hex-format IIPPRRXX.
 *
 * 		II	initial value
 * 		PP	polynomial
 * 		RR	reverse in & out (00 = false, 01 = true)
 * 		XX	value to xor the result with
 *
 * example:
 *
 * 	$ crc8 ff0c55ab000010
 *
 * 	lists crc-values in alphabetical order.
 *
 *
 * 	$ crc8 +ff0c55ab000010
 *
 * 	lists crc-values ordered by the crc-value.
 *
 *
 * 	$ crc8 ff0c55ab000010 00310000
 *
 * 	show the crc8-result of the crc8-recipe 00310000
 * 	    (init: 0x00, poly: 0x31, revio: false, xor: 0x00)
 *
 **/

#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define CRC8_IMPL
#include "crc8.h"

struct algo {
	struct crc8 recipe;
	char *name;
} algos[] = {
    { CRC8_AUTOSAR,    "Autosar"       },
    { CRC8_BLUETOOTH,  "Bluetooth"     },
    { CRC8_CDMA2000,   "CDMA-2000"     },
    { CRC8_DARC,       "DARC"          },
    { CRC8_DVBS2,      "DVB-S2"        },
    { CRC8_EBU,        "EBU"           },
    { CRC8_GSMA,       "GSM A"         },
    { CRC8_GSMB,       "GSM B"         },
    { CRC8_ICODE,      "I-Code"        },
    { CRC8_ITUI,       "ITU-I/I-432-1" },
    { CRC8_LTE,        "LTE"           },
    { CRC8_MAXIM,      "Maxim Dow"     },
    { CRC8_MIFAREMAD,  "Mifare Mad"    },
    { CRC8_NRSC5,      "NRSC5"         },
    { CRC8_OPENSAFETY, "OpenSafety"    },
    { CRC8_ROHC,       "ROHC"          },
    { CRC8_SAEJ1859,   "SAEJ1859"      },
    { CRC8_SMBUS,      "SMBUS/Plain"   },
    { CRC8_TECH3250,   "Tech3250"      },
    { CRC8_WCDMA,      "WCDMA"         } 
};
const unsigned algos_len = (sizeof(algos) / sizeof(*algos));

void die(const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);

	fputs("crc8_list: ", stderr);
	vfprintf(stderr, fmt, arg);
	fputc('\n', stderr);

	va_end(arg);
	
	exit(EXIT_FAILURE);
}
uint8_t hex_to_nibble(char ch)
{
	ch = toupper(ch);
	ch -= '0';
	if (ch > 9)
		ch -= 7;

	return (uint8_t)ch;
}
size_t dehexify(uint8_t *dst, const char *src)
{
	/**
	 * dehexifies hex-string @ src to dst (if dst != NULL).
	 *
	 * returns the number of bytes processed or -1 on error.
	 **/

	size_t len;
	uint8_t di;
	uint8_t si;

	len = strspn(src, "0123456789abcdefABCDEF");
	if (src[len] || len % 2)
		die("invalid hex-string");

	if (dst == NULL)
		return len;

	for (di = si = 0; src[si]; si += 2, di += 1) {
		dst[di] = hex_to_nibble(src[si]);
		dst[di] <<= 4;
		dst[di] |= hex_to_nibble(src[si + 1]);
	}

	return len;
}
void crc_all(const uint8_t *data, size_t len)
{
	int i;

	for (i = 0; i < algos_len; i ++) {
		struct crc8 *crc = &algos[i].recipe;
		crc8(crc, data, len);
		crc8_fin(crc);
	}
}
int algo_cmp_crc(const void *a, const void *b)
{
	struct crc8* ca = &((struct algo*)a)->recipe;
	struct crc8* cb = &((struct algo*)b)->recipe;

	return ca->crc - cb->crc;
}
uint8_t *parse_argument(char *argv[], size_t *len, int *sbc)
{
	char *hx;
	uint8_t *data;

	if ((hx = argv[1]) == NULL)
		die("Usage: crc8_list [+]hex-string [recipe IIPPRRXX]");

	if (argv[2]) {
		size_t len;
		len = dehexify((uint8_t*)&(*algos).recipe, argv[2]);
		if (len > 8)
			die("invalid crc8-recipe");
		crc8_init(&(*algos).recipe);
	}

	if (*hx == '+') {
		*sbc = 1;
		hx ++;
	}

	if (!strncmp(hx, "0x", 2))
		hx += 2;
	
	*len = dehexify(NULL, hx);
	if (*len % 2)
		die("invalid hex-string");

	*len /= 2;
	if (!(data = malloc(*len)))
		die("out of memory");

	if (dehexify(data, hx) == -1);

	return data;
}

int main(int argc, char* argv[])
{
	int i;
	size_t len;
	uint8_t *data;
	int sort_by_crc = 0;

	data = parse_argument(argv, &len, &sort_by_crc);

	if (argv[2]) {
		struct crc8 *crc = &algos[0].recipe;
		crc8(crc, data, len);
		crc8_fin(crc);
		printf("0x%02x\n", crc->crc); 
		exit(EXIT_SUCCESS);
	}

	crc_all(data, len);
	if (sort_by_crc)
		qsort(algos, algos_len, sizeof(*algos), algo_cmp_crc);
	
	for (i = 0; i < algos_len; i ++)
		printf("0x%02x  %s\n",
		    (unsigned)algos[i].recipe.crc, algos[i].name);

	return EXIT_SUCCESS;
}

