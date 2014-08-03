
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
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
	tlv_t * cur = NULL;

	for (i = 0; i < msg->nitems; i++) {
		cur = MSG_TLV(msg, i);
		if (NULL == cur->value) {
			continue;
		}

		if (TLV_TYPE_MSG == cur->type) {
			msg_free((message_t *)(cur->value));
		} else {
			free(cur->value);
		}
	}

	memset(msg, 0, sizeof(message_t) + msg->capacity * sizeof(tlv_t));
	free(msg);	
}

tlv_t * msg_append(message_t * msg, uint16_t type, uint16_t id, void * value, uint32_t length)
{
	tlv_t * newtlv = NULL;

	if ((NULL == msg) || (MSG_MAGIC != msg->magic) || (NULL == value)) {
		return NULL;
	}

	if ((0 == MSG_AVAILABLE_TLVS(msg)) || (0 == length)) {
		return NULL;
	}

	if (0 != validate_tlv_length(type, length)) {
		return NULL;
	}

	newtlv = MSG_LAST_TLV(msg);
	newtlv->type = type;
	newtlv->id = id;
	newtlv->length = length;

	if (TLV_TYPE_MSG == type) {
		newtlv->value = value;
	} else {
		newtlv->value = malloc(length);
		if (NULL == newtlv->value) {
			return NULL;
		}

		memcpy(newtlv->value, value, length);
	}

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
		psize += (sizeof(tlv_t) - sizeof(uint8_t*));
		if (TLV_TYPE_MSG == cur->type) {
			psize += msg_get_packed_size((message_t *)(cur->value));
		} else {
			psize += cur->length;
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
		printf("\ttype: %d\n", cur->type);
		printf("\tid: %d\n", cur->id);
		printf("\tsize: %d\n", cur->length);
		if (TLV_TYPE_MSG == cur->type) {
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
	uint16_t id = 0, length = 0, type = 0;
	int ret = 0;

	if ((NULL == packed) || (0 == size)) {
		return NULL;
	}

	if (MSG_MAGIC != ntohl(GET32(pos))) {
		return NULL;
	}
	ADVANCE32(pos);
	
	newmsg = msg_init(ntohl(GET32(pos)));
	if (NULL == newmsg) {
		return NULL;
	}
	ADVANCE32(pos);

	for (i = 0; i < (newmsg->capacity) && (pos - packed < size); i++) {
		id = ntohs(GET16(pos)); ADVANCE16(pos);
		type = ntohs(GET16(pos)); ADVANCE16(pos);
		length = ntohs(GET16(pos)); ADVANCE16(pos);
		cur = MSG_TLV(newmsg, i);
		ret = unpack_item(type, id, pos, length, cur);
		if (0 != ret) {
			msg_free(newmsg);
			return NULL;
		}

		pos += length;
	}
	newmsg->nitems = newmsg->capacity;

	return newmsg;
}

int msg_pack(message_t * msg, uint8_t * out, uint32_t * out_size)
{
	uint32_t psize = 0;
	uint32_t outsize = 0;
	uint32_t left = 0;
	uint32_t i = 0;
	tlv_t * cur = NULL;
	message_t * cur_msg = NULL;

	if ((NULL == msg) || (NULL == out) || (NULL == out_size)) {
		printf("something is null\n");
		return -1;
	}

	psize = msg_get_packed_size(msg);
	if (*out_size < psize) {
		return -1;
	}

	PUT32(out, htonl(MSG_MAGIC)); ADVANCE32(out);
	PUT32(out, htonl(msg->nitems)); ADVANCE32(out);
	left = (*out_size - (sizeof(uint32_t) * 2));
	for (i = 0; i < msg->nitems; i++) {
		cur = MSG_TLV(msg, i);
		PUT16(out, htons(cur->id)); ADVANCE16(out);
		PUT16(out, htons(cur->type)); ADVANCE16(out);
		if (TLV_TYPE_MSG == cur->type) {
			cur_msg = (message_t *)(cur->value);
			PUT16(out, htons(msg_get_packed_size(cur_msg))); ADVANCE16(out);
		} else {
			PUT16(out, htons(cur->length)); ADVANCE16(out);
		}
		left -= (sizeof(uint16_t) * 3);
		outsize = left;
		pack_item(cur, out, &outsize);
		out += outsize;
		left -= outsize;
	}

	*out_size = psize;
	return 0;
}
