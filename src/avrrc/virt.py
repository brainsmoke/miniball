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

import sys, os, time, cmath

import pygame

positions = [

(0.425919, 0.689152, -0.586227),
(0.498675, 0.308198, -0.810146),
(0.806873, 0.072756, -0.586227),
(0.924594, 0.308198, -0.223919),
(0.689152, 0.689152, -0.223919),
(0.380954, 0.924594, 0.000000),
(0.689152, 0.689152, 0.223919),
(0.924594, 0.308198, 0.223919),
(0.806873, 0.072756, 0.586227),
(0.498675, 0.308198, 0.810146),
(0.425919, 0.689152, 0.586227),
(0.117721, 0.924594, 0.362309),
(0.000000, 0.689152, 0.724617),
(0.072756, 0.308198, 0.948536),
(-0.308198, 0.072756, 0.948536),
(-0.616396, 0.308198, 0.724617),
(-0.425919, 0.689152, 0.586227),
(-0.308198, 0.924594, 0.223919),
(-0.689152, 0.689152, 0.223919),
(-0.879629, 0.308198, 0.362309),
(-0.997350, 0.072756, -0.000000),
(-0.879629, 0.308198, -0.362309),
(-0.689152, 0.689152, -0.223919),
(-0.308198, 0.924594, -0.223919),
(-0.425919, 0.689152, -0.586227),
(-0.616396, 0.308198, -0.724617),
(-0.308198, 0.072756, -0.948536),
(0.072756, 0.308198, -0.948536),
(0.000000, 0.689152, -0.724617),
(0.117721, 0.924594, -0.362309),

]

positions = positions + [ (-x,-y,z) for (x,y,z) in positions ]

def rotate( x, y, angle):
    z = (x + y*1j)*cmath.rect(1, angle)
    return (z.real, z.imag)

def flatten(coords, angle=0):
    return [ rotate(x, z, angle) for (x, y, z) in positions ]

black = (0, 0, 0)

class Metronome(object):
    def __init__(self, fps):
        self.delay = 1./fps

    def start(self):
        self.last = time.time()

    def sync(self):
        now = time.time()
        if self.delay > now-self.last:
            time.sleep(self.delay - (now-self.last))
        self.last = max(now, self.last+self.delay)

class VirtualScreen(object):

    def recalc(self, w, h ):
        self.pos = [ ( int((1+x*.9)*w/2), int((1+y*.9)*h/2) ) for (x,y) in self.positions ]

    def __init__(self, positions, windowsize=(500, 500)):
        pygame.init()
        self.screen = pygame.display.set_mode(windowsize, pygame.RESIZABLE | pygame.DOUBLEBUF)
        self.screen.fill(black)
        self.positions = positions
        min_dim = min(windowsize)
        self.recalc(min_dim, min_dim)
        self.r = min_dim / 10

    def draw_led(self, surf, x, y, r, color):
        midcolor = tuple( int(((x/255.)**.6) * 127.5) for x in color )
        ledcolor = tuple( min(255, int(((x/255.)**.6) * 765)) for x in color )
        for dim, bcolor in (.5, midcolor), (.25, ledcolor): 
             pygame.draw.circle(surf, bcolor, (x, y), int(r*dim))

    def draw(self):
        surf = pygame.display.get_surface()
        for i, (x, y) in enumerate(self.pos):
                self.draw_led(surf, x, y, self.r, self.buf[i*3:i*3+3])

    def check_events(self):
        for event in pygame.event.get():
            if event.type in (pygame.QUIT,):
                sys.exit(0)
            if event.type in (pygame.VIDEORESIZE,):
                self.screen = pygame.display.set_mode((event.w, event.h),
                        pygame.RESIZABLE | pygame.DOUBLEBUF)
                self.r = float(event.w) / 10
                self.recalc(event.w, event.h)

    def load_data(self, data):
        self.buf = data

    def push_data(self, data):
        self.load_data(data)
        self.push()

    def push(self):
        self.check_events()
        self.draw()
        pygame.display.flip()
        self.screen.fill(black)


pos = flatten(positions)
pos = pos[:30]+[ (2-x, y) for (x,y) in pos[30:] ]
s = VirtualScreen(windowsize=(640, 320), positions=pos)

fps = 400.
metronome = Metronome(fps)
metronome.start()

while True:
	d = sys.stdin.buffer.read(180)
	if len(d) == 0:
		break

	s.push_data(d)
	metronome.sync()
