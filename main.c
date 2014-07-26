
#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include "c-tlv.h"

struct person;
typedef struct person person_t;
typedef int (*struct_pack_t)(void * stru, uint8_t * outbuf, uint32_t * outsize);
typedef int (*struct_unpack_t)(uint8_t * inbuf, uint32_t size, void * outstruct);

typedef enum {
	ID_PERSON = 1,
	ID_PERSON_NAME,
	ID_PERSON_AGE,
	ID_PERSON_NKIDS,
	ID_PERSON_KIDS,
} items_id_t;

struct person {
	char * name;
	uint32_t age;
	uint32_t nkids;
	person_t * kids;
};

message_t * person_gen_msg(person_t * person)
{
	message_t * msg = NULL;
	message_t * kids = NULL;
	message_t * kid = NULL;
	int index = 0;

	msg = msg_init(4);
	if (NULL == msg) {
		return NULL;
	}

	kids = msg_init(person->nkids);
	if (NULL == kids) {
		msg_free(msg);
		return NULL;
	}

	for (; index < person->nkids; index++) {
		kid = person_gen_msg(&(person->kids[index]));
		if (NULL == kid) {
			msg_free(kids);
			msg_free(msg);
			return NULL;
		}
		msg_append(kids, TLV_TYPE_MSG, ID_PERSON, kid, MSG_SIZE(kid));
	}

	msg_append(msg, TLV_TYPE_BYTES, ID_PERSON_NAME, person->name, strlen(person->name));
	msg_append(msg, TLV_TYPE_UINT32, ID_PERSON_AGE, &(person->age), sizeof(person->age));
	msg_append(msg, TLV_TYPE_UINT32, ID_PERSON_NKIDS, &(person->nkids), sizeof(person->nkids));
	msg_append(msg, TLV_TYPE_MSG, ID_PERSON_KIDS, kids, MSG_SIZE(kids));

	return msg;
}

int person_unpack(uint8_t * inbuf, uint32_t size, person_t * outperson)
{
	return 0;
}

int person_init(person_t * person)
{
	memset(person, 0, sizeof(person_t));
	person->name = NULL;
	person->nkids = 0;
	person->kids = NULL;
	person->age = 0;
	return 0;
}

int dump_to_file(uint8_t * buf, uint32_t size, char * name)
{
	FILE * f = NULL;

	f = fopen(name, "wb");
	fwrite(buf, size, 1, f);
	fclose(f);

	return 0;
}

int main()
{
	person_t myperson;
	person_t kids[2];
	uint8_t packed[1500] = {0};
	uint32_t packed_size = sizeof(packed);
	message_t * msg = NULL;

	person_init(&kids[0]);
	kids[0].age = 3;
	kids[0].name = "kid0";

	person_init(&kids[1]);
	kids[1].age = 5;
	kids[1].name = "kid1";

	person_init(&myperson);
	myperson.name = "goniz";
	myperson.age = 21;
	myperson.kids = kids;
	myperson.nkids = 2;

	msg = person_gen_msg(&myperson);
	msg_print(msg);
	msg_pack(msg, packed, &packed_size);
	msg_free(msg);
	dump_to_file(packed, packed_size, "/tmp/output");
	return 0;
}

/*
int oldmain()
{
	message_t * msg = NULL;
	message_t * inner_msg = NULL;
	uint32_t value = 7;
	uint32_t psize = 0;
	uint8_t packed[1500] = { 0 };
	FILE * f = NULL;

	inner_msg = msg_init(1);
	msg_append(inner_msg, TLV_TYPE_BYTES, "INNER MSG", sizeof("INNER MSG"));

	msg = msg_init(7);
	msg_append(msg, TLV_TYPE_INT32, &value, sizeof(value));
	msg_append(msg, TLV_TYPE_INT32, &value, sizeof(value));
	msg_append(msg, TLV_TYPE_MSG, inner_msg, MSG_SIZE(inner_msg));
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
} */
