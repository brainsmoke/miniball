import sys, os

import pygame

import sync
import calcpos

black = (0, 0, 0)

class VirtualScreen(object):

    def recalc(self, w, h ):
        self.pos = [ ( int((1+x*.9)*w/2), int((1+y*.9)*h/2) ) for (x,y) in self.positions ]

    def __init__(self, positions, windowsize=(500, 500)):
        pygame.init()
        self.screen = pygame.display.set_mode(windowsize, pygame.RESIZABLE | pygame.DOUBLEBUF)
        self.screen.fill(black)
        self.positions = positions
        self.recalc(float(windowsize[0]), float(windowsize[1]))
        self.r = float(windowsize[0]) / 10

    def draw_led(self, surf, x, y, r, color):
        midcolor = tuple( int(((x/255.)**.6) * 127.5) for x in color )
        ledcolor = tuple( min(255, int(((x/255.)**.6) * 765)) for x in color )
        for dim, bcolor in (.5, midcolor), (.25, ledcolor): 
             pygame.draw.circle(surf, bcolor, (x, y), int(r*dim))

    def draw(self):
        surf = pygame.display.get_surface()
        for i, (x, y) in enumerate(self.pos):
                self.draw_led(surf, x, y, self.r, tuple(x for x in self.buf[i*3:i*3+3]))

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


class Fire:

    def color(self, x):
        r,g,b = (x**1*3, x**1.5*4., x**2)
        if r > 1.:
            r = 1.
        if g > 1.:
            g = 1.
        if b > 1.:
            b = 1.
        if (r, g, b) == (1., 1., 1.):
            r,g,b = 0.,0.,0.
        return bytes ( (int(r*255), int(g*255), int(b*255)) )

    def __init__(self, w, h):
        self.img = [ [ 0 ] * w for y in range(h+1) ]
        self.w, self.h = w, h
        self.mapping = [ int((x/64.)**1.7 / 12.7 * 64.) for x in range(255) ] + [ 63 ]
        #self.mapping = [ int((x/64.)**1.7 / 7.7 * 64.) for x in range(255) ] + [ 128 ]
        self.colortab = [ self.color(x/64.) for x in range(64) ] + [b'\0\0\0']*(256-64)
        self.it = 0
        self.map = [30, 29, 44, 51, 60, 58, 56, 50, 42, 33, 40, 24, 16, 25, 18, 10, 0, 2, 4, 11, 20, 13, 6, 15, 31, 39, 55, 62, 53, 38]

# /* https://www.avrfreaks.net/forum/tiny-fast-prng */
#uint8_t prng(void) {
#static uint8_t s=0xaa,a=0;
#
#        s^=s<<3;
#        s^=s>>5;
#        s^=a++>>2;
#        return s;
#}
        self.s = 0xaa
        self.a = 0
    def prng(self):
        self.s ^= self.s <<3
        self.s ^= self.s >>5
        self.a += 1
        self.s ^= self.a >> 2
        return self.s 

    def next(self):
        w, h = self.w, self.h

        self.it += 1
        if self.it % 8 == 0:
            r = self.prng()
            for x in range(w):
                self.img[h][x] = 44 * (r&1)
                r >>= 1

        for y in range(h):
            for x in range(w):
                s = 0
                for dx in (-1, 0, 1):
                    if 0 <= x+dx < w:
                        s += self.img[y+1][(x+dx)%w]
                self.img[y][x] = (self.img[y][x]>>1) + (self.img[y][x]>>2) + self.mapping[min(s,255)]
                #self.img[y][x] = (self.img[y][x]>>1) + self.mapping[min(s,255)]

        img = [ self.img[y][x] for y in range(self.h) for x in range(self.w) ]
        return b''.join( self.colortab[ img[ix] ] for ix in self.map )


if __name__ == '__main__':

    pos = calcpos.flatten(calcpos.positions)

    s = VirtualScreen(windowsize=(500, 500), positions=pos)
    a = Fire(8, 8)

    fps = 25.
    metronome = sync.Metronome(fps)
    metronome.start()

    while True:
        s.push_data(a.next())
        metronome.sync()
