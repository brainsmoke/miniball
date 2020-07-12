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
# 
# Partial tinyAVR reduced core emulator
#

import sys

DDRB=0x01
PORTB=0x02
CLKPSR=0x36
CCP=0x3C
SREG=0x3f

supported_IO_list = (DDRB, PORTB, CCP, SREG, CLKPSR)

sreg_c = 1<<0 # brsh on 0 / brlo on 1
sreg_z = 1<<1 # brne on 0 / breq on 1
sreg_n = 1<<2 # brpl on 0 / brmi on 1
sreg_v = 1<<3 # brvc on 0 / brvs on 1
sreg_s = 1<<4 # brge on 0 / btlt on 1
sreg_h = 1<<5 # brhc on 0 / brhs on 1
sreg_t = 1<<6 # brtc on 0 / brts on 1
sreg_i = 1<<7 # brid on 0 / brie on 1

def alu_sbc(dst, src, sreg):
    c, z = bool(sreg & sreg_c), bool(sreg & sreg_z)
    r = (dst - src - c)&0xff
    h = bool( ( ~dst&src | src&r | r&~dst ) & 1<<3 )
    v = bool( ( dst&~src&~r | ~dst&src&r ) & 1<<7 )
    n = bool( r & 1<<7 )
    z = bool( r == 0 ) & z
    s = n^v
    c = bool( ( ~dst&src | src&r | r&~dst ) & 1<<7 )
    sreg &= sreg_i|sreg_t
    sreg |= c<<0 | z<<1 | n<<2 | v<<3 | s<<4 | h<<5
    return (r, sreg)

def alu_adc(dst, src, sreg):
    c = bool(sreg & sreg_c)
    r = (dst + src + c)&0xff
    h = bool( ( dst&src | src&~r | ~r&dst ) & 1<<3 )
    v = bool( ( dst&src&~r | ~dst&~src&r ) & 1<<7 )
    n = bool( r & 1<<7 )
    z = bool( r == 0 ) #?
    s = n^v
    c = bool( ( dst&src | src&~r | ~r&dst ) & 1<<7 )
    sreg &= sreg_i|sreg_t
    sreg |= c<<0 | z<<1 | n<<2 | v<<3 | s<<4 | h<<5
    return (r, sreg)

def alu_sub(dst, src, sreg):
    return alu_sbc(dst, src, (sreg&~sreg_c)|sreg_z)

def alu_add(dst, src, sreg):
    return alu_adc(dst, src, sreg&~sreg_c)

def alu_cp(dst, src, sreg):
    r, sreg = alu_sub(dst, src, sreg)
    return (dst, sreg)

def alu_cpc(dst, src, sreg):
    r, sreg = alu_sbc(dst, src, sreg)
    return (dst, sreg)

def alu_bitwise(dst, src, sreg, op):
    r = op(dst, src)&0xff
    n = bool( r & 1<<7 )
    z = bool( r == 0 )
    v = False
    s = n^v
    sreg &= sreg_i|sreg_t|sreg_h|sreg_c
    sreg |= z<<1 | n<<2 | s<<4
    return (r, sreg)

def alu_and(dst, src, sreg):
    return alu_bitwise(dst, src, sreg, lambda dst,src : dst&src)

def alu_eor(dst, src, sreg):
    return alu_bitwise(dst, src, sreg, lambda dst,src : dst^src)

def alu_or(dst, src, sreg):
    return alu_bitwise(dst, src, sreg, lambda dst,src : dst|src)

def alu_mov(dst, src, sreg):
    return (src, sreg)

def alu_bug(dst, src, sreg):
    raise ValueError("alu_bug")

#
# VM
#

PROGMEM_ADDR = 0x4000
IO_BASE = 0

def watch_mem(ctx, addr, func):
    ctx['watch'][addr] = func

def watch_io(ctx, ioaddr, func):
    watch_mem(ctx, IO_BASE+ioaddr, func)

def eat_cycles(ctx, n_cycles):
    ctx['ts']+=n_cycles

def read_reg(ctx, reg):
    if 16 <= reg < 32:
        return ctx['regs'][reg]
    else:
        raise KeyError("bad register")

def write_reg(ctx, reg, val):
    if not 0 <= val < 0x100:
        raise ValueError(val)
    if 16 <= reg < 32:
        ctx['regs'][reg] = val
    else:
        raise KeyError("bad register")

def read_reg16(ctx, reg):
    if not reg & 1 and 16 <= reg < 32:
        return read_reg(ctx, reg) | read_reg(ctx, reg+1)<<8
    else:
        raise KeyError("bad register")

def write_reg16(ctx, reg, val):
    if not reg & 1 and 16 <= reg < 32:
        write_reg(ctx, reg, val&0xff)
        write_reg(ctx, reg+1, val>>8)
    else:
        raise KeyError("bad register")

def read_mem(ctx, addr):
    return ctx['mem'][addr]

def write_mem(ctx, addr, val):
    if addr not in ctx['mem']:
        raise KeyError("unknown location: "+hex(addr))
    if is_progmem(ctx, addr):
        raise KeyError("read-only location: "+hex(addr))
    orig = ctx['mem'][addr]
    ctx['mem'][addr] = val
    if addr in ctx['watch']:
        ctx['watch'][addr](ctx, addr, orig, val)

def is_progmem(ctx, addr):
    return addr >= PROGMEM_ADDR

def read_io(ctx, ioaddr):
    return read_mem(ctx, IO_BASE+ioaddr)

def write_io(ctx, ioaddr, val):
    write_mem(ctx, IO_BASE+ioaddr, val)

def next_op(ctx):
    op_func, op_size, op_args = ctx['code'][ctx['pc']]
    ctx['pc'] += op_size
    eat_cycles(ctx, op_size>>1)
    return op_func, op_size, op_args

def skip_op(ctx):
    next_op(ctx)

def reljump(ctx, reladdr):
    ctx['pc'] += reladdr
    eat_cycles(ctx, 1)

def op_unimpl(ctx):
    raise KeyError("unimplemented")

def op_nop(ctx):
    return 1

def op_alu(ctx, dst, src, alu):
    dst_v = read_reg(ctx, dst)
    src_v = read_reg(ctx, src)
    sreg = read_io(ctx, SREG)
    result, sreg = alu(dst_v, src_v, sreg)
    write_reg(ctx, dst, result)
    write_io(ctx, SREG, sreg)

def op_cpse(ctx, dst, src):
    dst_v = read_reg(ctx, dst)
    src_v = read_reg(ctx, src)
    if dst_v == src_v:
        skip_op(ctx)

def op_cpi(ctx, dst, imm):
    dst_v = read_reg(ctx, dst)
    sreg = read_io(ctx, SREG)
    _, sreg = alu_cp(dst_v, imm, sreg)
    write_io(ctx, SREG, sreg)

def op_subi(ctx, dst, imm):
    dst_v = read_reg(ctx, dst)
    sreg = read_io(ctx, SREG)
    result, sreg = alu_sub(dst_v, imm, sreg)
    write_reg(ctx, dst, result)
    write_io(ctx, SREG, sreg)

def op_ori(ctx, dst, imm):
    dst_v = read_reg(ctx, dst)
    sreg = read_io(ctx, SREG)
    result, sreg = alu_or(dst_v, imm, sreg)
    write_reg(ctx, dst, result)
    write_io(ctx, SREG, sreg)

def op_andi(ctx, dst, imm):
    dst_v = read_reg(ctx, dst)
    sreg = read_io(ctx, SREG)
    result, sreg = alu_and(dst_v, imm, sreg)
    write_reg(ctx, dst, result)
    write_io(ctx, SREG, sreg)

def op_inc(ctx, dst):
    dst_v = read_reg(ctx, dst)
    sreg = read_io(ctx, SREG)
    result, sreg_n = alu_add(dst_v, 1, sreg)
    sreg &= (sreg_c|sreg_h)
    sreg |=~(sreg_c|sreg_h) & sreg_n
    write_reg(ctx, dst, result)
    write_io(ctx, SREG, sreg)

def op_flagset(ctx, flag, val):
    sreg = read_io(ctx, SREG)
    sreg &=~ (1<<flag)
    sreg |= val<<flag
    write_io(ctx, SREG, sreg)

def op_dec(ctx, dst):
    dst_v = read_reg(ctx, dst)
    sreg = read_io(ctx, SREG)
    result, sreg_n = alu_sub(dst_v, 1, sreg)
    sreg &= (sreg_c|sreg_h)
    sreg |=~(sreg_c|sreg_h) & sreg_n
    write_reg(ctx, dst, result)
    write_io(ctx, SREG, sreg)

def op_lds16(ctx, dst, addr):
    val = read_mem(ctx, addr)
    write_reg(ctx, dst, val)

def op_sts16(ctx, src, addr):
    val = read_reg(ctx, src)
    write_mem(ctx, addr, val)

def op_in(ctx, dst, ioaddr):
    val = read_io(ctx, ioaddr)
    write_reg(ctx, dst, val)

def op_out(ctx, src, ioaddr):
    val = read_reg(ctx, src)
    write_io(ctx, ioaddr, val)

def op_rjmp(ctx, reladdr):
    reljump(ctx, reladdr)

def op_ldi(ctx, dst, imm):
    write_reg(ctx, dst, imm)

def op_brxx(ctx, val, cond, reladdr):
    sreg = read_io(ctx, SREG)
    if sreg>>cond & 1 == val:
        reljump(ctx, reladdr)

def op_sbrx(ctx, src, bit, pos):
    r = read_reg(ctx, src)
    if r>>pos & 1 == bit:
        skip_op(ctx)

def op_ld(ctx, dst, ptr, ptr_op): # guessed cycle counts from multiple sources and applying common sense
    if ptr == dst or ptr == dst+1:
        raise ValueError("ub")
    addr = read_reg16(ctx, ptr)
    extra_cycles = 0
    if ptr_op == -1:
        addr -= 1
        extra_cycles += 1
        write_reg16(ctx, ptr, addr)

    val = read_mem(ctx, addr)
    write_reg(ctx, dst, val)
    if is_progmem(ctx, addr) or ptr_op == 1:
        extra_cycles += 1

    if ptr_op == 1:
        addr += 1
        write_reg16(ctx, ptr, addr)
    eat_cycles(ctx, extra_cycles)

def op_st(ctx, src, ptr, ptr_op):
    if ptr == src or ptr == src+1:
        raise ValueError("ub")
    addr = read_reg16(ctx, ptr)
    extra_cycles = 0
    if ptr_op == -1:
        addr -= 1
        extra_cycles += 1
        write_reg16(ctx, ptr, addr)

    val = read_reg(ctx, src)
    write_mem(ctx, addr, val)

    if ptr_op == 1:
        addr += 1
        write_reg16(ctx, ptr, addr)
    eat_cycles(ctx, extra_cycles)
#
# Optable parsing
#

alu_map = {
#   '0000' : # nop/movw/mul*
    '0001' : alu_cpc,
    '0010' : alu_sbc,
    '0011' : alu_add,
#   '0100' : # cpse
    '0101' : alu_cp,
    '0110' : alu_sub,
    '0111' : alu_adc,
    '1000' : alu_and,
    '1001' : alu_eor,
    '1010' : alu_or,
    '1011' : alu_mov,
#   '11xx' : # cpi
}

def number(bitstring):
    return int(bitstring,2)

def signed_number(bitstring):
    return int(bitstring[1:],2) - ( int(bitstring[0]=='1')<<(len(bitstring)-1) )

def jump_relative(bitstring):
    return signed_number(bitstring)<<1

def reg_parse(bitstring):
    if len(bitstring) == 3:
        bitstring = '10'+bitstring
    elif len(bitstring) == 4:
        bitstring = '1'+bitstring
    elif len(bitstring) != 5:
        raise ValueError(bitstring)

    return number(bitstring)

def alu_parse(bitstring):
    return alu_map[bitstring]

def identity(bitstring):
    return bitstring

def inv_bit(bit):
    if len(bit) != 1:
        raise ValueError(bit)
    return 1-int(bit,2)

def bit(bit):
    if len(bit) != 1:
        raise ValueError(bit)
    return int(bit,2)

def load_store7(bitstring):
    return int(bitstring[2]=='0')*128 + int(bitstring[2]=='1')*64 + number(bitstring[:2]+bitstring[3:])

def ptr_y_or_z(bitstring):
    if bitstring == '1':
        return 28
    else:
        return 30

def ptr_x(bitstring):
    return 26

def ptr_op(bitstring):
    if bitstring == '00':
        return 0
    elif bitstring == '01':
        return 1
    elif bitstring == '10':
        return -1
    else:
        raise ValueError(bitstring)

fields = {
    '?' : ('unknown', identity      ),
    'U' : ('alu',     alu_parse     ),
    'd' : ('dst',     reg_parse     ),
    'r' : ('src',     reg_parse     ),
    'K' : ('imm',     number        ),
    'k' : ('addr',    load_store7   ),
    'j' : ('reladdr', jump_relative ),
    'i' : ('val',     inv_bit       ),
    'f' : ('flag',    number        ),
    'c' : ('cond',    number        ),
    'B' : ('bit',     bit           ),
    'b' : ('pos',     number        ),
    'A' : ('ioaddr',  number        ),
    'y' : ('ptr',     ptr_y_or_z    ),
    'x' : ('ptr',     ptr_x         ),
    'm' : ('ptr_op',  ptr_op        ),
}

opcodes = {
    '0000000000000000' : op_nop,
    '000000??????????' :            op_unimpl,
    '00UUUUrdddddrrrr' : op_alu,
    '000100rdddddrrrr' : op_cpse,
    '0011KKKKddddKKKK' : op_cpi,
    '0101KKKKddddKKKK' : op_subi,
    '0110KKKKddddKKKK' : op_ori,
    '0111KKKKddddKKKK' : op_andi,
    '1001010ddddd0011' : op_inc,
    '10010100ifff1000' : op_flagset,
    '1001010ddddd1010' : op_dec,

    '1000000dddddy0mm' : op_ld,
    '1000000dddddy001' :            op_unimpl,
    '1000000dddddy01?' :            op_unimpl,
    '1001000dddddy000' :            op_unimpl,
    '1001000dddddy0mm' : op_ld,
    '1001000ddddd0011' :            op_unimpl,
    '1001000ddddd01??' :            op_unimpl,
    '1001000dddddx1mm' : op_ld,
    '1001000ddddd1111' :            op_unimpl,

    '1000001rrrrry0mm' : op_st,
    '1000001rrrrry001' :            op_unimpl,
    '1000001rrrrry01?' :            op_unimpl,
    '1001001rrrrry000' :            op_unimpl,
    '1001001rrrrry0mm' : op_st,
    '1001001rrrrr0011' :            op_unimpl,
    '1001001rrrrr01??' :            op_unimpl,
    '1001001rrrrrx1mm' : op_st,
    '1001001rrrrr1111' :            op_unimpl,

    '10100kkkddddkkkk' : op_lds16,
    '10101kkkrrrrkkkk' : op_sts16,
    '10110AAdddddAAAA' : op_in,
    '10111AArrrrrAAAA' : op_out,
    '1100jjjjjjjjjjjj' : op_rjmp,
    '1110KKKKddddKKKK' : op_ldi,
    '11110ijjjjjjjccc' : op_brxx,
    '111111Brrrrr0bbb' : op_sbrx,
    '????????????????' :            op_unimpl,
}

def parse_opcode(mem, addr, opcodes, fields):
    code = mem[addr] | mem[addr+1]<<8
    bitfield = '{:016b}'.format(code)
    best = -1
    argbest = None
    for patt, op_func in opcodes.items():
        n_matches = 0
        field_patterns = {}
        for p,b in zip(patt,bitfield):
            if p == b:
                n_matches += 1
            elif p in '01' and b in '01':
                n_matches = -1
                break
            else:
                if p not in field_patterns:
                    field_patterns[p] = ''
                field_patterns[p] += b
        if n_matches > best:
            best = n_matches
            argbest = ( field_patterns, op_func )

    field_patterns, op_func = argbest
    return (op_func, 2, { fields[k][0]:fields[k][1](v) for k, v in field_patterns.items() })

def init_ctx(flash):
    regs = [None]*16+[0]*16
    code = { x : parse_opcode(flash, x, opcodes, fields) for x in flash.keys() if not x & 1 and x+1 in flash }
    mem = { x:0 for x in range(0x40, 0x60) }

    for i,v in flash.items():
        mem[PROGMEM_ADDR+i] = v

    for i in supported_IO_list:
        mem[IO_BASE+i] = 0

    return { 'regs':regs, 'mem':mem, 'pc':0, 'ts':0, 'code': code, 'watch': {}}

def run(ctx, n_cycles=-1):
    while n_cycles==-1 or ctx['ts'] < n_cycles:
        #print(' '*50,'{:02x}'.format(read_mem(ctx, SREG)), '[ '+' '.join('{:02x}'.format(x) for x in ctx['regs'][16:])+' ]')
        op_func, op_size, op_args = next_op(ctx)
        #print (ctx['ts'], hex(ctx['pc']), op_func.__name__, op_args)
        op_func(ctx, **op_args)

