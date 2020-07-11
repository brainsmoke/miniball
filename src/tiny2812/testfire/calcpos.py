import cmath

#miniball
positions = [
[0.689152, 0.689152, -0.223919],

[0.380954, 0.924594, 0.000000],

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

[0.425919, 0.689152, -0.586227],
[0.498675, 0.308198, -0.810146],
[0.806873, 0.072756, -0.586227],
[0.924594, 0.308198, -0.223919],

[0.924594, 0.308198, 0.223919],
[0.806873, 0.072756, 0.586227],
[0.498675, 0.308198, 0.810146],
[0.425919, 0.689152, 0.586227],
[0.689152, 0.689152, 0.223919],
]

def d2(a, b):
    ax, ay = a
    bx, by = b
    dx, dy = ax-bx, ay-by
    return dx*dx + dy*dy

# 'pull in' corners
def grid_to_circle_projection(dim):
        proj = [None] * 64
        dim = int(dim)
        for y in range(dim):
            dy = y*2-(dim-1)
            for x in range(dim):
                dx = x*2-(dim-1)
                i = x + y * dim
                if dx == 0 and dy == 0:
                    lx, ly = 1, 1
                elif abs(dy) < abs(dx):
                    lx, ly = float(dim), float(dy)/dx*dim
                else:
                    lx, ly = float(dx)/dy*dim, float(dim)
                r = 1/(lx*lx + ly*ly)**.5
                nx, ny = dx*r, dy*r
                proj[i] = (nx,ny)

        return proj

def rotate( x, y, angle):
    z = (x + y*1j)*cmath.rect(1, angle)
    return (z.real, z.imag)

def flatten(coords, angle=0):
    return [ rotate(x, z, angle) for (x, y, z) in positions ]

def map_to_nearest(from_, to):
    x = [None] * len(from_) 
    m = [None] * len(from_) 

    for ia, a in enumerate(from_):
        for ib, b in enumerate(to):
            d = d2(a,b)
            if x[ia] == None or x[ia] > d:
                x[ia] = d
                m[ia] = ib

    return m

def mapping(angle = 0, dim=8):
	return map_to_nearest(flatten(positions, angle), grid_to_circle_projection(dim))

if __name__ == '__main__':
    print (mapping())
