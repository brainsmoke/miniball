#!/usr/bin/env python3
#
# Copyright (c) 2020 Erik Bosman <erik@minemu.org>
#
# Permission  is  hereby  granted,  free  of  charge,  to  any  person
# obtaining  a copy  of  this  software  and  associated documentation
# files (the "Software"),  to deal in the Software without restriction,
# including  without  limitation  the  rights  to  use,  copy,  modify,
# merge, publish, distribute, sublicense, and/or sell copies of the
# Software,  and to permit persons to whom the Software is furnished to
# do so, subject to the following conditions:
#
# The  above  copyright  notice  and this  permission  notice  shall be
# included  in  all  copies  or  substantial portions  of the Software.
#
# THE SOFTWARE  IS  PROVIDED  "AS IS", WITHOUT WARRANTY  OF ANY KIND,
# EXPRESS OR IMPLIED,  INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY,  FITNESS  FOR  A  PARTICULAR  PURPOSE  AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM,  DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT,  TORT OR OTHERWISE,  ARISING FROM, OUT OF OR IN
# CONNECTION  WITH THE SOFTWARE  OR THE USE  OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# (http://opensource.org/licenses/mit-license.html)
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

