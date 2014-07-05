#ifndef __C_TLV_H__
#define __C_TLV_H__

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define UINT8(ptr)					((uint8_t *)(ptr))
#define UINT16(ptr)					((uint16_t *)(ptr))
#define UINT32(ptr)					((uint32_t *)(ptr))

#define PACKED						__attribute__((packed))
#define PUT8(ptr, c)				(*UINT8((ptr)) = (uint8_t)(c))
#define PUT16(ptr, s)				(*UINT16((ptr)) = (uint16_t)(s))
#define PUT32(ptr, l)				(*UINT32((ptr)) = (uint32_t)(l))

#define GET8(ptr)					(*UINT8((ptr)))
#define GET16(ptr)					(*UINT16((ptr)))
#define GET32(ptr)					(*UINT32((ptr)))

#define ADVANCE8(ptr)				ptr = (UINT8(UINT8((ptr)) + 1))
#define ADVANCE16(ptr)				ptr = (UINT8(UINT16((ptr)) + 1))
#define ADVANCE32(ptr)				ptr = (UINT8(UINT32((ptr)) + 1))

typedef struct {
	uint16_t id;
	uint16_t size;
	uint8_t is_msg;
	uint8_t * value;
} PACKED tlv_t;

typedef struct {
	uint32_t magic;
	uint32_t nitems;
	uint32_t capacity;
	tlv_t tlvs[0];
} PACKED message_t;

#define MSG_MAGIC					(0x12345678)
#define MSG_AVAILABLE_TLVS(msg)		(((msg)->capacity) - ((msg)->nitems))
#define MSG_TLV(msg, index)			(&((msg)->tlvs[(index)]))
#define MSG_LAST_TLV(msg)			(MSG_TLV((msg), (msg)->nitems))
#define MSG_SIZE(msg)				(sizeof(message_t) + sizeof(tlv_t) * (msg)->capacity)

message_t * msg_init(uint32_t max_items);
void msg_free(message_t * msg);
tlv_t * msg_append(message_t * msg, uint32_t id, void * value, uint32_t length);
uint32_t msg_get_packed_size(message_t * msg);
void msg_print(message_t * msg);
message_t * msg_unpack(uint8_t * packed, uint32_t size);
int msg_pack(message_t * msg, uint8_t * out, uint32_t * out_size);

#endif
