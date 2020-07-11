#!/usr/bin/env python3
# 
# Intel HEX file parser
#

def parse(s):
	base_addr = 0
	mem = {}
	end = False
	for record in s.split('\n'):
		if record[0:1] == ':':
			if end:
				raise 'meh.'
			b = bytes.fromhex(record[1:])
			if sum(b) & 0xff == 0:
				n_bytes = b[0]
				addr16 = b[1]<<8 | b[2]
				rec_type = b[3]
				data = b[4:-1]
				if n_bytes != len(data):
					raise ValueError(record)
				if rec_type == 0:
					for i,v in enumerate(data):
						mem[base_addr+addr16+i] = v
				elif rec_type == 1:
					end = True
				elif rec_type == 2:
					if len(data) != 2:
						raise ValueError(record)
					base_addr = ( data[1]<<8 | data[2] ) <<  4
				elif rec_type == 4:
					if len(data) != 2:
						raise ValueError(record)
					base_addr = ( data[1]<<8 | data[2] ) << 16
				else:
					raise ValueError('rec_type \''+str(rec_type)+'\' unsupported')
			else:
				raise ValueError(record)
	return mem

