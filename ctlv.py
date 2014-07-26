#!/usr/bin/python2

from StringIO import StringIO
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

class TlvID(object):
	INT8 	= 7
	UINT8 	= 8
	INT16	= 9
	UINT16	= 10
	INT32	= 11
	UINT32	= 12
	BYTES	= 13
	MSG		= 14
	

class ItemParser(object):
	def __init__(self):
		self.parsers = dict()
		self.parsers[TlvID.INT8] = self.parse_int8
		self.parsers[TlvID.UINT8] = self.parse_uint8
		self.parsers[TlvID.INT16] = self.parse_int16
		self.parsers[TlvID.UINT16] = self.parse_uint16
		self.parsers[TlvID.INT32] = self.parse_int32
		self.parsers[TlvID.UINT32] = self.parse_uint32
		self.parsers[TlvID.BYTES] = self.parse_bytes
		self.parsers[TlvID.MSG] = self.parse_msg

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

	def parse_item(self, itemid, data):
		parser = self.parsers[itemid]
		return parser(data)

class Message(object):
	MAGIC = 0x12345678

	def __init__(self):
		self.items = []

	def pack(self):
		pass

	def get_packed_size(self):
		pass

	def __str__(self):
		return ' '.join(map(str, self.items))

	@classmethod
	def unpack(cls, buf):
		stream = Buffer(buf)
		parser = ItemParser()
		msg = Message()
		magic = stream.read_int32()
		errmsg = 'Missing magic from buffer. expected %s, found %s' % (hex(Message.MAGIC), hex(magic))
		assert magic == Message.MAGIC, errmsg
		nitems = stream.read_int32()
		for index in xrange(nitems):
			itemid = stream.read_int16()
			size = stream.read_int16()
			item_data = stream.read(size)
			msg.items += [parser.parse_item(itemid, item_data)]
		leftover = stream.read()
		if 0 != len(leftover):
			print 'found %d trailing bytes while parsing' % (len(leftover), )
		return msg

def main():
	packed = open(sys.argv[1], 'rb').read()
	msg = Message.unpack(packed)
	print msg
	return 0

if __name__ == '__main__':
	sys.exit(main())
