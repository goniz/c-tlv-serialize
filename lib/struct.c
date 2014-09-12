
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include "c-tlv.h"

int pack_int8(uint8_t * int8, uint8_t * outbuf, uint32_t * outsize)
{
	if ((NULL == int8) || (NULL == outbuf) || (NULL == outsize)) {
		return -1;
	}

	if (1 > *outsize) {
		return -1;
	}

	*outsize = 1;
	*outbuf = *int8;
	return 0;
}

int pack_int16(uint16_t * int16, uint16_t * outbuf, uint32_t * outsize)
{
	if ((NULL == int16) || (NULL == outbuf) || (NULL == outsize)) {
		return -1;
	}

	if (2 > *outsize) {
		return -1;
	}

	*outsize = 2;
	*outbuf = htons(*int16);
	return 0;
}

int unpack_int16(uint16_t * int16, uint16_t * outbuf, uint32_t * outsize)
{
	if ((NULL == int16) || (NULL == outbuf) || (NULL == outsize)) {
		return -1;
	}

	if (2 > *outsize) {
		return -1;
	}

	*outsize = 2;
	*outbuf = ntohs(*int16);
	return 0;
}

int pack_int32(uint32_t * int32, uint32_t * outbuf, uint32_t * outsize)
{
	if ((NULL == int32) || (NULL == outbuf) || (NULL == outsize)) {
		return -1;
	}

	if (4 > *outsize) {
		return -1;
	}

	*outsize = 4;
	*outbuf = htonl(*int32);
	return 0;
}

int unpack_int32(uint32_t * int32, uint32_t * outbuf, uint32_t * outsize)
{
	if ((NULL == int32) || (NULL == outbuf) || (NULL == outsize)) {
		return -1;
	}

	if (4 > *outsize) {
		return -1;
	}

	*outsize = 4;
	*outbuf = ntohl(*int32);
	return 0;
}

int pack_bytes(void * inbuf, uint32_t insize, void * outbuf, uint32_t * outsize)
{
	if ((NULL == inbuf) || (NULL == outbuf) || (NULL == outsize)) {
		return -1;
	}

	if (insize > *outsize) {
		return -1;
	}

	*outsize = insize;
	memcpy(outbuf, inbuf, insize);
	return 0;
}

int pack_item(tlv_t * item, void * outbuf, uint32_t * outsize)
{
	int ret = 0;

	if ((NULL == item) || (NULL == outbuf) || (NULL == outsize)) {
		return -1;
	}

	switch (item->type) {
		case TLV_TYPE_INT8:
		case TLV_TYPE_UINT8:
			ret = pack_int8((uint8_t *)(item->value), outbuf, outsize);
			break;
		case TLV_TYPE_INT16:
		case TLV_TYPE_UINT16:
			ret = pack_int16((uint16_t *)(item->value), outbuf, outsize);
			break;
		case TLV_TYPE_INT32:
		case TLV_TYPE_UINT32:
			ret = pack_int32((uint32_t *)(item->value), outbuf, outsize);
			break;
		case TLV_TYPE_BYTES:
			ret = pack_bytes(item->value, item->length, outbuf, outsize);
			break;
		case TLV_TYPE_MSG:
			ret = msg_pack((message_t *)(item->value), outbuf, outsize);
			break;
		default:
			ret = -1;
			break;
	}

	return ret;
}

int unpack_item(tlv_type_t type, uint16_t id, void * inbuf, uint16_t length, tlv_t * outtlv)
{
	uint32_t outsize = 0;
	int ret = 0;
	void * value = NULL;
	message_t * newmsg = NULL;

	if ((NULL == inbuf) || (NULL == outtlv)) {
		return -1;
	}

	value = malloc(length);
	if (NULL == value) {
		return -1;
	}

	switch (type) {
		case TLV_TYPE_INT8:
		case TLV_TYPE_UINT8:
			outsize = length;
			ret = pack_int8((uint8_t *)inbuf, (uint8_t *)value, &outsize);
			break;

		case TLV_TYPE_INT16:
		case TLV_TYPE_UINT16:
			outsize = length;
			ret = unpack_int16((uint16_t *)inbuf, (uint16_t *)value, &outsize);
			break;

		case TLV_TYPE_INT32:
		case TLV_TYPE_UINT32:
			outsize = length;
			ret = unpack_int32((uint32_t *)inbuf, (uint32_t *)value, &outsize);
			break;

		case TLV_TYPE_BYTES:
			outsize = length;
			ret = pack_bytes(inbuf, length, value, &outsize);
			break;

		case TLV_TYPE_MSG:
			newmsg = msg_unpack((uint8_t *)inbuf, length);
			if (NULL == newmsg) {
				free(value);
				ret = -1;
				goto exit;
			}

			memcpy(value, newmsg, MSG_SIZE(newmsg));
			free(newmsg);
			break;

		default:
			ret = -1;
	}

	if (0 != ret) {
		free(value);
		goto exit;
	}

	outtlv->id = id;
	outtlv->type = type;
	outtlv->length = length;
	outtlv->value = value;

exit:
	return ret;
}

int validate_tlv_length(tlv_type_t type, uint32_t length)
{
	int ret = 0;

	switch (type) {
		case TLV_TYPE_INT8:
		case TLV_TYPE_UINT8:
			ret = (1 != length);
			break;
		case TLV_TYPE_INT16:
		case TLV_TYPE_UINT16:
			ret = (2 != length);
			break;
		case TLV_TYPE_INT32:
		case TLV_TYPE_UINT32:
			ret = (4 != length);
			break;
		case TLV_TYPE_BYTES:
			ret = 0;
			break;
		case TLV_TYPE_MSG:
			ret = 0;
			break;
		default:
			ret = -1;
			break;
	}

	return ret;
}
