#include "decoder.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

static int reed32_callback(r_device *decoder, bitbuffer_t *bitbuffer) {
    bitbuffer_invert(bitbuffer);
    //printf("\ndecode start");
    for(int y = 0; y < 5; ++y) {
        unsigned bits = bitbuffer->bits_per_row[y];
        if (bits != 49) continue;
        //printf("%d", y);
        uint8_t *b = bitbuffer->bb[y];

        int id = b[0];

	//printf("a: %d %d %d %d\n",id, b[0], b[1], b[2]);
        if (id == 0 || id == 0x7fff)
            continue; // reject min/max to reduce false positives

        if ((id & 16) != 16)
            continue;

	if ((id & 32) != 0)
	    continue;

        uint32_t reed_ul;
        uint8_t validate = id;
        uint8_t check = b[5];
        char reed[22];

        reed_ul = (b[1] << 24) + (b[2] << 16) + (b[3] << 8) + b[4];

        validate ^= b[1] ^ b[2] ^ b[3] ^ b[4];

        if(validate != check)
            continue;

        snprintf(reed, 22, "%"PRIu32"", reed_ul);


    	/* clang-format off */
   	data_t *data = data_make(
    		"model" ,    	"",             	DATA_STRING, "Reed32",
            "id",        	"ID",             	DATA_INT, id,
            "reed"   ,      "Reed switch",    DATA_STRING, reed,
            NULL);
    	/* clang-format on */
    	decoder_output_data(decoder, data);

    	return 1;
    }
    return DECODE_ABORT_EARLY;
}

static char *output_fields[] = {
		"model",
        "id",
        "reed",
        NULL,
};

r_device reed32 = {
        .name        = "DIY reed switch counter",
        .modulation  = OOK_PULSE_PWM,
        .short_width = 316,          // Threshold between short and long pulse [us]
        .long_width  = 692,         // Maximum gap size before new row of bits [us]
        .gap_limit   = 0,  // Maximum gap size before new row of bits [us]
        .reset_limit = 6228, // Maximum gap size before End Of Message [us]
	.sync_width  = 412,
        .tolerance   = 200,
        .decode_fn   = &reed32_callback,
        .fields      = output_fields,
};
