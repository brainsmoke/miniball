#include <avr/io.h>

#define rI         r16
#define rJ         r17
#define rK         r18
#define rData      r19 
#define rRem       r20
#define rHigh      r21

#define rTmp       r22

#define rIlow      r23
#define addrJlow   RAMSTART
#define addrKlow   (RAMSTART+1)
#define rLoop      r24
#define rOut       r25

#define pPhaseBuf  X
#define loPhaseBuf r26
#define hiPhaseBuf r27

#define pWave      Y
#define loWave     r28
#define hiWave     r29

#define pRemPtr    Z
#define loRemPtr   r30
#define rZero      r31     /* hi(RAMEND) == 0 */

#define REMBUFSTART (RAMSTART+ 2)
#define REMBUFEND   (RAMSTART+32)

#define OUTPORT PORTB
#define OUTDDR DDRB
#define OUTPIN  2
#define BRIGHTNESS_MASK 0x0f
#define REMAINDER_MASK (0xff-BRIGHTNESS_MASK)

.section .text

.global main

.org 0x0

init:
ldi loWave, lo8(wave+__AVR_TINY_PM_BASE_ADDRESS__)
ldi hiWave, hi8(wave+__AVR_TINY_PM_BASE_ADDRESS__)

ldi rTmp, _BV(OUTPIN)
out _SFR_IO_ADDR(OUTDDR), rTmp
ldi rTmp, 0xD8
out _SFR_IO_ADDR(CCP), rTmp
ldi rZero, 0
out _SFR_IO_ADDR(CLKPSR), rZero

next_frame:
subi rIlow, 44
adc rI, rZero
lds rTmp, addrJlow
subi rTmp, 52
adc rJ, rZero
sts addrJlow, rTmp
lds rTmp, addrKlow
subi rTmp, 60
adc rK, rZero
sts addrKlow, rTmp

//wait loop
ldi rTmp, 13
idle_outer:
ldi rRem, 140
idle_inner:
dec rRem
brne idle_inner
dec rTmp
brne idle_outer
nop

bitbang:

ldi loPhaseBuf, lo8(phase+__AVR_TINY_PM_BASE_ADDRESS__)
ldi hiPhaseBuf, hi8(phase+__AVR_TINY_PM_BASE_ADDRESS__)
ldi rLoop, 240
ldi rHigh, 0
ldi loRemPtr, REMBUFSTART

/* pseudo code for computation part:
 *
 * rLoop = 240;
 * do
 * {
 *     uint8_t index = rI + *pPhaseBuf++;
 *     if (index < 0x80) { index = 0x80; }
 *     rData = wave[-0x80+index];            // brightness value [0..240], nibbles swapped
 *     if (pRemPtr == &RemBuf[30]) { pRemPtr = &RemBuf[0]; }
 *     rRem = *pRemPtr;                      // 4-bit remainder value in high nibble
 *     rData += rRem; (remember carry bit c) //
 *     rData += c;                           // brightness value [0..255], nibbles swapped
 *     *pRemPtr = rRem & 0xf0                // store 4-bit remainder again
 *     if (rLoop & 2) { pRemPtr++ ; rLoop-- ; }
 *     rLoop--;
 *     (rI, rJ, rK) = (rJ, rK, rI);
 * }
 * while (rLoop > 0);
 */

bitbang_loop:
/* 0 +  1  */ out _SFR_IO_ADDR(OUTPORT), rHigh
/* 1 +  2  */                                      ld loWave, pPhaseBuf+
/* 3 +  1  */ out _SFR_IO_ADDR(OUTPORT), rZero
/* 4 +  1  */                                      cpi loRemPtr, REMBUFEND
/* 5 + 1|2 */                                      brne 1f
/* 6 + 1|0 */                                      ldi loRemPtr, REMBUFSTART
1:
/* 7 +  1  */                                      ld rRem, pRemPtr
/* 8 +  1  */                                      add loWave, rI
/* 9 +  1  */                                                                   mov rTmp, rI


/* 0 +  1  */ out _SFR_IO_ADDR(OUTPORT), rHigh
/* 1 + 1|2 */                                      brmi 1f
/* 2 + 1|0 */                                      ldi loWave, lo8(wave+__AVR_TINY_PM_BASE_ADDRESS__)
1:
/* 3 +  1  */ out _SFR_IO_ADDR(OUTPORT), rZero
/* 4 +  1  */                                                                   mov rI, rJ
/* 5 +  1  */                                      ld rData, pWave
/* 7 +  1  */                                      add rRem, rData
/* 8 +  1  */                                      adc rData, rZero
/* 9 +  1  */                                      andi rRem, REMAINDER_MASK

/* 0 +  1  */ out _SFR_IO_ADDR(OUTPORT), rHigh
/* 1 +  1  */                                      st pRemPtr, rRem
/* 2 + 1|2 */ sbrs rOut, 5
/* 3 + 1|0 */ out _SFR_IO_ADDR(OUTPORT), rZero
/* 4 + 1|2 */                                      sbrc rLoop, 1
/* 5 + 1|0 */                                      inc loRemPtr
/* 6 +  1  */                                                                   mov rJ, rK
/* 7 +  1  */ out _SFR_IO_ADDR(OUTPORT), rZero
/* 8 + 1|2 */                                      sbrc rLoop, 1
/* 9 + 1|0 */                                      dec rLoop

/* 0 +  1  */ out _SFR_IO_ADDR(OUTPORT), rHigh
/* 1 +  1  */ ldi rRem, 4
/* 2 + 1|2 */ sbrs rOut, 4
/* 3 + 1|0 */ out _SFR_IO_ADDR(OUTPORT), rZero
/* 4 +  1  */                                                                   mov rK, rTmp
/* 5 +  1  */ mov rTmp, rOut
0:
/* 6 +  1  */ mov rOut, rData
/* 7 +  1  */ out _SFR_IO_ADDR(OUTPORT), rZero
/* 8 +  1  */ lsl rTmp
/* 9 +  1  */                                      andi rOut, BRIGHTNESS_MASK

/* 0 +  1  */ out _SFR_IO_ADDR(OUTPORT), rHigh
/* 1 +  1  */ dec rRem
/* 2 + 1|2 */ sbrs rTmp, 4
/* 3 + 1|0 */ out _SFR_IO_ADDR(OUTPORT), rZero
/* 4 + 1|2 */ brne 0b
/* 5 +  1  */ ldi rHigh, _BV(OUTPIN)
/* 6 +  1  */                                      subi rLoop, 1
/* 7 +  1  */ out _SFR_IO_ADDR(OUTPORT), rZero
/* 8 + 1|2 */                                      brcc bitbang_loop
mov rTmp, rK
mov rK, rJ
mov rJ, rI
mov rI, rTmp
rjmp next_frame

phase:

/*
import cmath
position = [
[0.425919, 0.689152, -0.586227],
[0.498675, 0.308198, -0.810146],
[0.806873, 0.072756, -0.586227],
[0.924594, 0.308198, -0.223919],
[0.689152, 0.689152, -0.223919],
[0.380954, 0.924594, 0.000000],
[0.689152, 0.689152, 0.223919],
[0.924594, 0.308198, 0.223919],
[0.806873, 0.072756, 0.586227],
[0.498675, 0.308198, 0.810146],
[0.425919, 0.689152, 0.586227],
[0.117721, 0.924594, 0.362309],
[0.000000, 0.689152, 0.724617],
[0.072756, 0.308198, 0.948536],
[-0.308198, 0.072756, 0.948536],
[-0.616396, 0.308198, 0.724617],
[-0.425919, 0.689152, 0.586227],
[-0.308198, 0.924594, 0.223919],
[-0.689152, 0.689152, 0.223919],
[-0.879629, 0.308198, 0.362309],
[-0.997350, 0.072756, -0.000000],
[-0.879629, 0.308198, -0.362309],
[-0.689152, 0.689152, -0.223919],
[-0.308198, 0.924594, -0.223919],
[-0.425919, 0.689152, -0.586227],
[-0.616396, 0.308198, -0.724617],
[-0.308198, 0.072756, -0.948536],
[0.072756, 0.308198, -0.948536],
[0.000000, 0.689152, -0.724617],
[0.117721, 0.924594, -0.362309],
]
position = position + [ (-x,-y,z) for x,y,z in position ]
rot_pos = []
rot = cmath.rect(1, cmath.pi/4)
for x, y, z in position:
    q = complex(x, y)*rot
    x, y = q.real, q.imag
    q = complex(y, z)*rot
    y, z = q.real, q.imag
    rot_pos += [[x, y, z]]

xaxis = [ int(256*(cmath.phase(complex(y,z))/cmath.pi/2+.5)) for x, y, z in rot_pos ]
yaxis = [ int(256*(cmath.phase(complex(z,x))/cmath.pi/2+.5)) for x, y, z in rot_pos ]
zaxis = [ int(256*(cmath.phase(complex(x,y))/cmath.pi/2+.5)) for x, y, z in rot_pos ]

arr = [ ','.join( (str(i),str(j),str(k)) ) for (i,j,k) in zip(xaxis,yaxis,zaxis) ]

print ('.byte '+', '.join(arr))
 */
.byte 133,90,199, 120,228,186, 129,190,169, 149,158,171, 150,128,192, 160,106,213, 169,128,192, 170,148,161, 190,150,129, 199,133,91, 186,120,229, 178,102,238, 199,106,13, 212,120,50, 231,109,50, 235,84,32, 211,88,14, 179,84,249, 224,70,6, 2,62,23, 32,41,22, 61,40,1, 96,57,249, 140,70,235, 108,49,231, 84,31,235, 88,13,210, 107,13,199, 120,50,213, 141,81,217, 58,248,101, 71,5,219, 62,22,1, 42,20,33, 41,0,63, 32,234,85, 22,0,63, 21,30,43, 1,62,41, 248,100,58, 5,218,71, 13,209,89, 248,178,85, 235,141,71, 216,141,82, 212,159,107, 236,177,103, 12,198,107, 224,185,121, 189,168,129, 160,169,150, 130,190,151, 95,198,134, 51,212,121, 83,216,142, 107,212,160, 103,237,178, 84,248,178, 71,234,141, 50,230,110

.p2align 7
wave:
/*
 * from math import pi, cos
 * max_val=0xf0
 * gamma=2.2
 * factor = max_val / (255.**gamma)
 * gamma_map = [ int(x**gamma * factor) for x in range(256) ]
 * gamma_map = [ (x<<4)&0xf0 | (x>>4)&0x0f for x in gamma_map ]
 * print (', '.join(str(gamma_map[int(-cos(x*pi/64)*127.5+127.5)]) for x in range(128) ))
 */

.byte 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 16, 32, 48, 64, 80, 96, 128, 160, 192, 224, 17, 65, 129, 177, 241, 66, 146, 226, 51, 147, 243, 100, 196, 53, 165, 38, 166, 39, 151, 40, 152, 25, 153, 26, 138, 250, 139, 219, 76, 172, 13, 93, 189, 253, 62, 126, 158, 190, 222, 222, 254, 222, 222, 190, 158, 126, 62, 253, 189, 93, 13, 172, 76, 219, 139, 250, 138, 26, 153, 25, 152, 40, 151, 39, 166, 38, 165, 53, 196, 100, 243, 147, 51, 226, 146, 66, 241, 177, 129, 65, 17, 224, 192, 160, 128, 96, 80, 64, 48, 32, 16, 16, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
