
#include "c-tlv.h"

char * g_items_id_enum[] = { 	SX(ID_PERSON),
								SX(ID_PERSON_NAME),
								SX(ID_PERSON_AGE),
								SX(ID_PERSON_NKIDS),
								SX(ID_PERSON_KIDS)
							};

char * g_tlv_type_enum[] = {
                                SX(TLV_TYPE_INT8),  SX(TLV_TYPE_UINT8),
                                SX(TLV_TYPE_INT16), SX(TLV_TYPE_UINT16),
                                SX(TLV_TYPE_INT32), SX(TLV_TYPE_UINT32),
                                SX(TLV_TYPE_BYTES),
                                SX(TLV_TYPE_MSG)
                            };