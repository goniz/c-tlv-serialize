
#include <stdio.h>
#include <stdint.h>
#include "c-tlv.h"

int main()
{
	message_t * msg = NULL;
	message_t * inner_msg = NULL;
	tlv_t * tlv = NULL;
	uint32_t value = 7;
	uint32_t psize = 0;
	char packed[1500] = { 0 };
	FILE * f = NULL;

	inner_msg = msg_init(1);
	msg_append(inner_msg, 1, "INNER MSG", sizeof("INNER MSG"));

	msg = msg_init(7);
	tlv = msg_append(msg, 5, &value, sizeof(value));
	tlv = msg_append(msg, 2, &value, sizeof(value));
	tlv = msg_append(msg, 1, inner_msg, MSG_SIZE(inner_msg));
	tlv->is_msg = 1;
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
