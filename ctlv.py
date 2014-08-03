#!/usr/bin/python2

from StringIO import StringIO
from pprint import pprint
import sys
import struct


class Buffer(StringIO):
    def read_uint32(self):
        b = self.read(4)
        return struct.unpack('!I', b)[0]

    def read_int32(self):
        b = self.read(4)
        return struct.unpack('!i', b)[0]

    def read_int64(self):
        b = self.read(8)
        return struct.unpack('!q', b)[0]

    def read_uint64(self):
        b = self.read(8)
        return struct.unpack('!Q', b)[0]

    def read_int16(self):
        b = self.read(2)
        return struct.unpack('!h', b)[0]

    def read_uint16(self):
        b = self.read(2)
        return struct.unpack('!H', b)[0]

    def read_int8(self):
        b = self.read(1)
        return struct.unpack('!b', b)[0]

    def read_uint8(self):
        b = self.read(1)
        return struct.unpack('!B', b)[0]


class TlvType(object):
    INT8 = 7
    UINT8 = 8
    INT16 = 9
    UINT16 = 10
    INT32 = 11
    UINT32 = 12
    BYTES = 13
    MSG = 14


class ItemParser(object):
    def __init__(self):
        self.parsers = dict()
        self.parsers[TlvType.INT8] = self.parse_int8
        self.parsers[TlvType.UINT8] = self.parse_uint8
        self.parsers[TlvType.INT16] = self.parse_int16
        self.parsers[TlvType.UINT16] = self.parse_uint16
        self.parsers[TlvType.INT32] = self.parse_int32
        self.parsers[TlvType.UINT32] = self.parse_uint32
        self.parsers[TlvType.BYTES] = self.parse_bytes
        self.parsers[TlvType.MSG] = self.parse_msg

    def parse_msg(self, data):
        return Message.unpack(data)

    def parse_int8(self, data):
        return Buffer(data).read_int8()

    def parse_uint8(self, data):
        return Buffer(data).read_uint8()

    def parse_int16(self, data):
        return Buffer(data).read_int16()

    def parse_uint16(self, data):
        return Buffer(data).read_uint16()

    def parse_int32(self, data):
        return Buffer(data).read_int32()

    def parse_uint32(self, data):
        return Buffer(data).read_uint32()

    def parse_bytes(self, data):
        return data

    def parse_item(self, item_type, data):
        parser = self.parsers[item_type]
        return parser(data)


class ItemID(dict):
    ID_PERSON = 1
    ID_PERSON_NAME = 2
    ID_PERSON_AGE = 3
    ID_PERSON_NKIDS = 4
    ID_PERSON_KIDS = 5

    def __init__(self):
        super(ItemID, self).__init__()
        self[ItemID.ID_PERSON] = "Person"
        self[ItemID.ID_PERSON_NAME] = "Name"
        self[ItemID.ID_PERSON_AGE] = "Age"
        self[ItemID.ID_PERSON_NKIDS] = "#kids"
        self[ItemID.ID_PERSON_KIDS] = "Kids"


class Message(dict):
    MAGIC = 0x12345678

    def pack(self):
        pass

    def get_packed_size(self):
        pass

    @classmethod
    def unpack(cls, buf):
        stream = Buffer(buf)
        parser = ItemParser()
        id_parser = ItemID()
        msg = Message()
        magic = stream.read_int32()
        errmsg = 'Missing magic from buffer. expected %s, found %s' % (hex(Message.MAGIC), hex(magic))
        assert magic == Message.MAGIC, errmsg
        nitems = stream.read_int32()
        print 'Parsing msg with %d items' % (nitems, )
        for index in xrange(nitems):
            itemid = stream.read_int16()
            itemid = id_parser[itemid]
            item_type = stream.read_int16()
            size = stream.read_int16()
            item_data = stream.read(size)
            msg[itemid] = parser.parse_item(item_type, item_data)
        leftover = stream.read()
        if 0 != len(leftover):
            print 'found %d trailing bytes while parsing' % (len(leftover), )
        return msg


def main():
    packed = open(sys.argv[1], 'rb').read()
    msg = Message.unpack(packed)
    pprint(msg)
    return 0

if __name__ == '__main__':
    sys.exit(main())
