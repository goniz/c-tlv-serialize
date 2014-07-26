
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "c-tlv.h"

int main()
{
	message_t * msg = NULL;
	message_t * inner_msg = NULL;
	uint32_t value = 7;
	uint32_t psize = 0;
	uint8_t packed[1500] = { 0 };
	FILE * f = NULL;

	inner_msg = msg_init(1);
	msg_append(inner_msg, TLV_ID_BYTES, "INNER MSG", sizeof("INNER MSG"));

	msg = msg_init(7);
	msg_append(msg, TLV_ID_INT32, &value, sizeof(value));
	msg_append(msg, TLV_ID_INT32, &value, sizeof(value));
	msg_append(msg, TLV_ID_MSG, inner_msg, MSG_SIZE(inner_msg));
	msg_print(msg);
	psize = msg_get_packed_size(msg); 
	printf("psize: %d\n", psize);
	psize = sizeof(packed);
	printf("ret: %d\n", msg_pack(msg, packed, &psize));
	free(inner_msg);
	msg_free(msg);

	f = fopen("/tmp/output", "wb");
	fwrite(packed, psize, 1, f);
	fclose(f);

	memset(packed, 0, sizeof(packed));
	f = fopen("/tmp/output", "rb");
	printf("ret: %d\n", fread(packed, 1, 50, f));
	fclose(f);

	msg = msg_unpack(packed, sizeof(packed));
	msg_print(msg);
	msg_free(msg);
	

	return 0;
}
