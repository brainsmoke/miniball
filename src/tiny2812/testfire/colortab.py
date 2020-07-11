
def color(x):
    r,g,b = (x**1*3, x**1.5*3., x**2)
    if r > 1.:
        r = 1.
    if g > 1.:
        g = 1.
    if b > 1.:
        b = 1.
    if (r, g, b) == (1., 1., 1.):
        r,g,b = 0.,0.,0.
    return hex(int(g*255))+','+hex(int(r*255))+','+hex(int(b*255))+','

print(', '.join( [color(x/64.) for x in xrange(64) ] + ['0,0,0']*(256-64)))


