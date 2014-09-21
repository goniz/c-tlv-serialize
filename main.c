
#include <stdio.h>
#include <stdint.h>
#include "c-tlv.h"

#define offsetof(type, member)  __builtin_offsetof (type, member)

struct person;
typedef struct person person_t;


struct serialize_obj;
typedef struct serialize_obj serialize_obj_t;

struct serialize_obj {
	items_id_t id;
	tlv_type_t type;
	char * name;
	serialize_obj_t * num;
	uint8_t * data;
	uint32_t size;
};
#define INIT_SERIALIZABLE_OBJ(obj, i, t, na, nu, d, s)										\
															(obj).id = (i);					\
															(obj).type = (t);				\
															(obj).name = (na);				\
															(obj).num = (nu);				\
															(obj).data = (uint8_t *)(d);	\
															(obj).size = (s)
#define SERIALIZABLE_MAGIC	  (0x87654321)
typedef struct {
	uint32_t magic;
	uint32_t num;
	serialize_obj_t * objs;
} serializeable_t;

typedef struct {
	serializeable_t __serialize;
} serializeable_object_t;

struct person {
	serializeable_t __serialize;
	char * name;
	uint32_t age;
	uint8_t nkids;
	person_t ** kids;
};

int person_init(person_t * person)
{
	serializeable_t * handle = NULL;
	uint32_t obj_size = 0;

	memset(person, 0, sizeof(person_t));
	person->name = NULL;
	person->nkids = 0;
	person->kids = NULL;
	person->age = 0;


	handle = &(person->__serialize);
	handle->num = 4;
	obj_size = sizeof(serialize_obj_t) * handle->num;
	handle->objs = malloc(obj_size);
	if (NULL == handle->objs) {
		return -1;
	}

	memset(handle->objs, 0, obj_size);
	handle->magic = SERIALIZABLE_MAGIC;
	INIT_SERIALIZABLE_OBJ(handle->objs[0], ID_PERSON_NAME, TLV_TYPE_STRING, "name", NULL, &(person->name), 20);
	INIT_SERIALIZABLE_OBJ(handle->objs[1], ID_PERSON_AGE, TLV_TYPE_UINT32, "age", NULL, &(person->age), sizeof(person->age));
	INIT_SERIALIZABLE_OBJ(handle->objs[2], ID_PERSON_NKIDS, TLV_TYPE_UINT8, "nkids", NULL, &(person->nkids), sizeof(person->nkids));
	INIT_SERIALIZABLE_OBJ(handle->objs[3], ID_PERSON_KIDS, TLV_TYPE_MSG, "kids", &(handle->objs[2]), &(person->kids), sizeof(person->kids));

	return 0;
}

void person_free(person_t * person)
{
	serializeable_t * handle = NULL;

	if (NULL == person) {
		return;
	}

	handle = &(person->__serialize);
	if (NULL != handle->objs) {
		free(handle->objs);
		handle->objs = NULL;
	}

	handle->magic = 0;	
}

message_t * struct_to_msg(serializeable_object_t * obj)
{
	uint32_t i = 0, j = 0;
	uint32_t nitems = 0;
	serializeable_object_t ** items = NULL;
	serializeable_t * handle = &(obj->__serialize);
	serialize_obj_t * current = NULL;
	serialize_obj_t * nitems_obj = NULL;
	message_t * msg = NULL;
	message_t * submsg = NULL;
	message_t * tmpmsg = NULL;

	if (SERIALIZABLE_MAGIC != handle->magic) {
		return NULL;
	}

	msg = msg_init(handle->num);
	if (NULL == msg) {
		return NULL;
	}

	for (i = 0; i < handle->num; i++) {
		current = &(handle->objs[i]);
		if (NULL == current) {
			goto error;
		}

		if (TLV_TYPE_MSG == current->type) {
			nitems_obj = current->num;
			if ((nitems_obj) && (TLV_TYPE_UINT8 != nitems_obj->type)) {
				goto error;
			}

			/* if its null, there is no nitems.. so assume its only 1 instance */
			nitems = nitems_obj != NULL ? GET8(nitems_obj->data) : 1;
			submsg = msg_init(nitems);
			if (NULL == submsg) {
				goto error;
			}

			items = *((serializeable_object_t ***)(current->data));
			if (NULL == items) {
				continue;
			}

			for (j = 0; j < submsg->capacity; j++) {
				serializeable_object_t * item = items[j];
				if (NULL == item) {
					break;
				}

				tmpmsg = struct_to_msg(item);
				if (NULL == tmpmsg) {
					msg_free(submsg);
					goto error;
				}

				msg_append(submsg, TLV_TYPE_MSG, current->id, tmpmsg, MSG_SIZE(tmpmsg));
			}

			msg_append(msg, current->type, current->id, submsg, MSG_SIZE(submsg));
		} else if (TLV_TYPE_BYTES == current->type) {
			char * data = *((char **)(current->data));
			msg_append(msg, current->type, current->id, data, current->size);
		} else if (TLV_TYPE_STRING == current->type) {
			char * data = *((char **)(current->data));
			msg_append(msg, current->type, current->id, data, strlen(data));
		} else {
			msg_append(msg, current->type, current->id, current->data, current->size);
		}
	}

	return msg;

error:
	printf("error\n");
	msg_free(msg);
	return NULL;
}

int dump_to_file(uint8_t * buf, uint32_t size, char * name)
{
	FILE * f = NULL;

	f = fopen(name, "wb");
	fwrite(buf, size, 1, f);
	fclose(f);

	return 0;
}

int read_from_file(uint8_t * buf, uint32_t size, char * name)
{
	FILE * f = NULL;

	memset(buf, 0, size);
	f = fopen(name, "rb");
	fread(buf, size, 1, f);
	fclose(f);

	return 0;
}

int main()
{
	person_t myperson;
	person_t kid0;
	person_t kid1;
	person_t * kids[2] = { &kid0, &kid1 };
	uint8_t packed[1500] = {0};
	uint32_t packed_size = sizeof(packed);
	message_t * msg = NULL;
	int ret = 0;

	ret = person_init(&kid0);
	if (0 != ret) {
		printf("person_init failed\n");
		return 1;
	}

	kid0.age = 3;
	kid0.name = "kid0";

	ret = person_init(&kid1);
	if (0 != ret) {
		printf("person_init failed\n");
		return 1;
	}

	kid1.age = 5;
	kid1.name = "kid1";

	ret = person_init(&myperson);
	if (0 != ret) {
		printf("person_init failed\n");
		return 1;
	}

	myperson.name = "goniz";
	myperson.age = 21;
	myperson.kids = kids;
	myperson.nkids = 2;

	msg = struct_to_msg((serializeable_object_t *)&myperson);
	if (NULL == msg) {
		printf("shit\n");
		return 1;
	}

	msg_print(msg);
	msg_pack(msg, packed, &packed_size);
	msg_free(msg);

	person_free(&kid0);
	person_free(&kid1);
	person_free(&myperson);
	dump_to_file(packed, packed_size, "/tmp/output");
	return 0;
}

