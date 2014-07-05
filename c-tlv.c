
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include "c-tlv.h"

message_t * msg_init(uint32_t max_items)
{
	uint32_t size = sizeof(message_t) + max_items * sizeof(tlv_t);
	message_t * out = malloc(size);
	
	if (NULL == out) {
		return NULL;
	}

	out->magic = MSG_MAGIC;
	out->nitems = 0;
	out->capacity = max_items;
	return out;
}

void msg_free(message_t * msg)
{
	uint32_t i = 0;

	for (i = 0; i < msg->nitems; i++) {
		free(msg->tlvs[i].value);
	}

	memset(msg, 0, sizeof(message_t) + msg->capacity * sizeof(tlv_t));
	free(msg);	
}

tlv_t * msg_append(message_t * msg, uint32_t id, void * value, uint32_t length)
{
	tlv_t * newtlv = NULL;

	if ((NULL == msg) || (MSG_MAGIC != msg->magic) || (NULL == value)) {
		return NULL;
	}

	if ((0 == MSG_AVAILABLE_TLVS(msg)) || (0 == length)) {
		return NULL;
	}

	newtlv = MSG_LAST_TLV(msg);
	newtlv->id = id;
	newtlv->is_msg = 0;
	newtlv->size = length;
	newtlv->value = malloc(length);
	if (NULL == newtlv->value) {
		return NULL;
	}

	memcpy(newtlv->value, value, length);
	msg->nitems++;

	return newtlv;
}

uint32_t msg_get_packed_size(message_t * msg)
{
	uint32_t psize = 0;
	uint32_t i = 0;

	psize = sizeof(uint32_t) + sizeof(uint32_t);
	for (i = 0; i < msg->nitems; i++) {
		tlv_t * cur = MSG_TLV(msg, i);
		if (cur->is_msg == 1) {
			psize += msg_get_packed_size((message_t *)(cur->value));
		} else {
			psize += (cur->size + sizeof(tlv_t) - sizeof(uint8_t*));
		}
	}

	return psize;
}

void msg_print(message_t * msg)
{
	uint32_t i = 0;
	tlv_t * cur = NULL;

	if (NULL == msg) {
		return;
	}

	printf("magic: %x\n", msg->magic);
	printf("nitems: %d\n", msg->nitems);
	printf("capacity: %d\n", msg->capacity);
	printf("tlvs:\n");

	for (i = 0; i < msg->nitems; i++) {
		cur = &(msg->tlvs[i]);
		printf("tlv index %d\n", i);
		printf("\tid: %d\n", cur->id);
		printf("\tsize: %d\n", cur->size);
		printf("\tis msg: %s\n", cur->is_msg == 0 ? "no" : "yes");
		if (cur->is_msg == 1) {
			msg_print((message_t *)(cur->value));
		} else {
			printf("\tvalue: %p %x\n", cur->value, *((uint32_t *)cur->value));
		}
	}

}

message_t * msg_unpack(uint8_t * packed, uint32_t size)
{
	uint32_t i = 0;
	uint8_t * pos = packed;
	message_t * newmsg = NULL;
	tlv_t * cur = NULL;
	uint16_t id, length = 0;
	uint8_t is_msg = 0;

	if ((NULL == packed) || (0 == size)) {
		return NULL;
	}

	if (MSG_MAGIC != GET32(pos)) {
		return NULL;
	}
	ADVANCE32(pos);
	
	newmsg = msg_init(GET32(pos));
	if (NULL == newmsg) {
		return NULL;
	}
	ADVANCE32(pos);

	for (i = 0; i < (newmsg->capacity) && (pos - packed < size); i++) {
		id = GET16(pos); ADVANCE16(pos);
		length = GET16(pos); ADVANCE16(pos);
		is_msg = GET8(pos); ADVANCE8(pos);
		if (is_msg == 1) {
			message_t * tmpmsg = msg_unpack(pos, length);
			if (NULL == tmpmsg) {
				msg_free(newmsg);
				return NULL;
			}

			cur = msg_append(newmsg, id, tmpmsg, MSG_SIZE(tmpmsg));
			if (NULL == cur) {
				msg_free(tmpmsg);
				msg_free(newmsg);
				return NULL;
			}
			cur->is_msg = 1;
			free(tmpmsg);
		} else {
			cur = msg_append(newmsg, id, pos, length);
			if (NULL == cur) {
				msg_free(newmsg);
				return NULL;
			}
		}

		pos += length;
	}

	return newmsg;
}

int msg_pack(message_t * msg, uint8_t * out, uint32_t * out_size)
{
	uint32_t psize = 0;
	uint32_t i = 0;
	tlv_t * cur = NULL;

	if ((NULL == msg) || (NULL == out) || (NULL == out_size)) {
		printf("something is null\n");
		return -1;
	}

	psize = msg_get_packed_size(msg);
	if (*out_size < psize) {
		printf("*out_size > psize\n");
		return -1;
	}

	PUT32(out, MSG_MAGIC); ADVANCE32(out);
	PUT32(out, msg->nitems); ADVANCE32(out);
	for (i = 0; i < msg->nitems; i++) {
		cur = MSG_TLV(msg, i);
		PUT16(out, cur->id); ADVANCE16(out);
		PUT16(out, cur->size); ADVANCE16(out);
		PUT8(out, cur->is_msg); ADVANCE8(out);
		if (cur->is_msg == 1) {
			message_t * tmpmsg  = (message_t *)(cur->value);
			uint32_t outsize = msg_get_packed_size(tmpmsg);
			msg_pack(tmpmsg, out, &outsize);
			out += outsize;
		} else {
			memcpy(out, cur->value, cur->size);
			out += cur->size;
		}
	}

	*out_size = psize;
	return 0;
}
