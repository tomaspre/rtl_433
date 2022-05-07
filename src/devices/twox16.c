#include "decoder.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

static int _2x16_callback(r_device *decoder, bitbuffer_t *bitbuffer) {
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

        if ((id & 32) != 32)
            continue;

	if ((id & 16) != 0)
	    continue;

        double  temp;
	double  volt;
        uint8_t validate = id;
        uint8_t check = b[5];

        volt = ((b[1] << 8) + b[2])/1000.0;
	temp = (((b[3] << 8) + b[4])/100.0)-100;

        validate ^= b[1] ^ b[2] ^ b[3] ^ b[4];

        if(validate != check)
            continue;


    	/* clang-format off */
   	data_t *data = data_make(
    		"model" ,    	"",             	DATA_STRING, "2x16",
            "id",        	"ID",             	DATA_INT,  id,
            "volt"   ,      "Voltage",    DATA_DOUBLE, volt,
	    "temp",         "Temperature", DATA_DOUBLE, temp,
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
        "volt",
	"temp",
        NULL,
};

r_device twox16 = {
        .name        = "DIY 2x16",
        .modulation  = OOK_PULSE_PWM,
        .short_width = 316,          // Threshold between short and long pulse [us]
        .long_width  = 692,         // Maximum gap size before new row of bits [us]
        .gap_limit   = 0,  // Maximum gap size before new row of bits [us]
        .reset_limit = 6228, // Maximum gap size before End Of Message [us]
	.sync_width  = 412,
        .tolerance   = 200,
        .decode_fn   = &_2x16_callback,
        .fields      = output_fields,
};
