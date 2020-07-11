#!/usr/bin/env python3
# 

import sys
import intelhex, avrrc

PIN = 2

def frame_timing(frame, idle_cycles, frame_cycles):
    print()
    print('cycles/frame+pause: {:d}, idle time: {:d}'.format(frame_cycles, idle_cycles))

def byte_hex(byte):
    print ('{:02x} '.format(byte), end='')

def frame_stdout(frame, idle_cycles, frame_cycles):
    sys.stdout.buffer.write(bytes(frame))
    sys.stdout.flush()

def wave_func(pin, byte_func=None, frame_func=None):

    last_up=-1
    last_frame=-1
    byte=0
    bits=0
    frame = []

    def func(ctx, addr, old, new):
        nonlocal last_up, last_frame, bits, byte, frame
        old = old>>pin & 1
        new = new>>pin &1
        if old ^ new:
            ts=ctx['ts']
            d = ts-last_up
            if new:
                if d == 10:
                    pass
                elif last_frame >= 0 and d < 2400:
                    raise ValueError("wrong timing: timeout too short? "+str(d))
                elif bits != 0:
                    raise ValueError("incomplete byte")
                else:
                    if last_frame >= 0:
                        if frame_func:
                            frame_func(frame, ts-last_up-10, ts-last_frame)
                        frame = []
                    last_frame = ts

                last_up=ts
                first = False
            elif old:
                if d not in (3,7):
                    raise ValueError("wrong timing: "+str(d))
                byte = byte<<1 | int( d == 7 )
                bits += 1
                if bits == 8:
                    if byte_func:
                        byte_func(byte)
                    frame.append(byte)
                    byte = 0
                    bits = 0
    return func

if __name__ == '__main__':

    if sys.argv[1:2] == ['-out']:
        func = wave_func(PIN, None, frame_stdout)
    elif sys.argv[1:2] == ['-timing']:
        func = wave_func(PIN, byte_hex, frame_timing)
    else:
        print ("Usage: {} -out|-timing < hexfile".format(sys.argv[0]), file=sys.stderr)
        sys.exit(1)

    flash = intelhex.parse(sys.stdin.read())

    ctx = avrrc.init_ctx(flash)
    avrrc.watch_io(ctx, avrrc.PORTB, func)

    avrrc.run(ctx)
