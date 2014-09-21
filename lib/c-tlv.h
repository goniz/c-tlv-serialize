#ifndef __C_TLV_H__
#define __C_TLV_H__

#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define S(s)						#s
#define SX(s)						S(s)
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

/* Person enum */
extern char * g_items_id_enum[];
typedef enum {
	ID_PERSON = 1,
	ID_PERSON_NAME,
	ID_PERSON_AGE,
	ID_PERSON_NKIDS,
	ID_PERSON_KIDS,
} items_id_t;
#define ITEM_ID_STR(e)				(g_items_id_enum[(e)-ID_PERSON])

/* Tlv enum */
extern char * g_tlv_type_enum[];
typedef enum {
	TLV_TYPE_INT8 	= 7,
	TLV_TYPE_UINT8 	= 8,
	TLV_TYPE_INT16	= 9,
	TLV_TYPE_UINT16	= 10,
	TLV_TYPE_INT32	= 11,
	TLV_TYPE_UINT32	= 12,
	TLV_TYPE_BYTES	= 13,
	TLV_TYPE_MSG	= 14,
	TLV_TYPE_STRING = 15,

} tlv_type_t;
#define TLV_TYPE_STR(e)             (g_tlv_type_enum[(e)-TLV_TYPE_INT8])

/* tlv/msg structures */
typedef struct {
	uint16_t id;
	uint16_t type;
	uint16_t length;
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
#define MSG_SIZE(msg)				((uint16_t)(sizeof(message_t) + sizeof(tlv_t) * (msg)->capacity))

message_t * msg_init(uint32_t max_items);
void msg_free(message_t * msg);
tlv_t * msg_append(message_t * msg, tlv_type_t id, uint16_t type, void * value, uint16_t length);
uint32_t msg_get_packed_size(message_t * msg);
void msg_print(message_t * msg);
void msg_pprint(message_t * msg, int depth);
message_t * msg_unpack(uint8_t * packed, uint32_t size);
int msg_pack(message_t * msg, uint8_t * out, uint32_t * out_size);
int pack_item(tlv_t * item, void * outbuf, uint32_t * outsize);
int unpack_item(tlv_type_t type, uint16_t id, void * inbuf, uint16_t length, tlv_t * outtlv);
int validate_tlv_length(tlv_type_t type, uint32_t length);
void print_item(tlv_t * item, int * depth);

#endif
