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
 * calculates multiple crc-values of a string of data represented in hex.
 *
 * usage: crc8_list [+]hexadecimal-string
 *
 * 	+	the list will be ordered incremental by crc-value. otherwise
 * 		it's ordered in alphabetical order.
 *
 * example:
 *
 * 	$ crc8_list ff0c55ab000010
 *
 * 	lists crc-values in alphabetical order.
 *
 * 	$ crc8_list +ff0c55ab000010
 *
 * 	lists crc-values ordered by the crc-value.
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

	fputs("crc8: ", stderr);
	vfprintf(stderr, fmt, arg);

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
void dehexify(uint8_t *dst, const char *src)
{
	uint8_t di, si;

	for (di = si = 0; src[si]; si += 2, di += 1) {
		dst[di] = hex_to_nibble(src[si]);
		dst[di] <<= 4;
		dst[di] |= hex_to_nibble(src[si + 1]);
	}
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
uint8_t *parse_argument(const char *arg, size_t *len, int *sbc)
{
	uint8_t *data;

	if (arg == NULL)
		die("Usage: crc8 [+]hex-string\n");

	if (*arg == '+') {
		*sbc = 1;
		arg ++;
	}

	if (!strncmp(arg, "0x", 2))
		arg += 2;
	
	*len = strspn(arg, "0123456789abcdefABCDEF");
	if (arg[*len])
		die("invalid character @ %i (%c)\n", *len, arg[*len]);
	if (*len % 2)
		die("crc8: uneven number of characters\n");

	*len /= 2;
	if (!(data = malloc(*len)))
		die("crc8: out of memory\n");

	dehexify(data, arg);

	return data;
}

int main(int argc, char* argv[])
{
	int i;
	size_t len;
	uint8_t *data;
	int sort_by_crc;

	data = parse_argument(argv[1], &len, &sort_by_crc);

	crc_all(data, len);
	if (sort_by_crc)
		qsort(algos, algos_len, sizeof(*algos), algo_cmp_crc);
	
	for (i = 0; i < algos_len; i ++)
		printf("0x%02x  %s\n",
		    (unsigned)algos[i].recipe.crc, algos[i].name);

	return EXIT_SUCCESS;
}

