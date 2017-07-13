#!/usr/bin/python

import sys, math, itertools

cubic_sections = 32

in_dpi, out_dpi = 90., 10000.
scale = out_dpi/in_dpi
new_scale = 25400/in_dpi

def roundint(x):
	return int(round(x))

def dist(a, b):
	ax, ay = a
	bx, by = b
	return math.sqrt((ax-bx)**2 + (ay-by)**2)

def interpolate(pos1, pos2, d):
	x1, y1 = pos1
	x2, y2 = pos2
	return ( x1*(1-d) + x2*d, y1*(1-d) + y2*d )

def vector_add(a, b):
	return tuple( i+j for i, j in zip(a,b) )

def vector_sub(a, b):
	return tuple( i-j for i, j in zip(a,b) )

def cubic_spline( start, guide1, guide2, end ):
	n = min(int(dist(start, end)*scale/40.)+1, cubic_sections)

	v = []
	for i in xrange(1, n+1):
		d = i/float(n)
		a = interpolate(start, guide1, d)
		b = interpolate(guide1, guide2, d)
		c = interpolate(guide2, end, d)

		ab = interpolate(a, b, d)
		bc = interpolate(b, c, d)
		abc = interpolate(ab, bc, d)
		v.append(abc)
	return v

def get_coords(s):
	return map(float, s)

def path_to_polygons(data):

	values = (x for x in data.replace(',', ' ').replace('\n',' ').split(' ') if x != '' )

	mode = 'z'
	pos = (0.,0.)

	polygons = []
	p = []

	for x in values:

		if x in 'zZ':
			pos = p[0]
			p.append( pos )

		if x in 'zZmM':
			if len(p) > 0:
				polygons.append( p )
				p = []

		if x in 'zZmclMCLhHvV':
			mode = x
			continue

		if mode == 'm':
			mode = 'l'

		if mode == 'M':
			mode = 'L'

		if mode == 'l':
			pos = vector_add(pos, get_coords((x, values.next())))
			p.append( pos )

		elif mode == 'L':
			pos = get_coords((x, values.next()))
			p.append( pos )

		elif mode == 'H':
			pos = ( float(x), pos[1] )
			p.append( pos )

		elif mode == 'V':
			pos = ( pos[0], float(x) )
			p.append( pos )

		elif mode == 'h':
			pos = vector_add(pos, [float(x), 0])
			p.append( pos )

		elif mode == 'v':
			pos = vector_add(pos, [0, float(x)])
			p.append( pos )

		elif mode in 'cC':
			start  = pos
			guide1 = get_coords( (x, values.next()) )
			guide2 = get_coords( (values.next(), values.next()) )
			end    = get_coords( (values.next(), values.next()) )

			if mode == 'c':
				guide1 = vector_add(pos, guide1)
				guide2 = vector_add(pos, guide2)
				end    = vector_add(pos, end)

			pos = end
			p.extend( cubic_spline(start, guide1, guide2, end) )
		else:
			print "ERROR: " + x
			sys.exit(1)

	if len(p) > 0:
		polygons.append( p )

	return polygons

def rescale_point(p, scale, conv=lambda x: x):
	x, y = p
	return ( conv(x*scale), conv(y*scale) )

def rescale_polygon(polygon, scale, conv=lambda x: x):
	return [ rescale_point(p, scale, conv) for p in polygon ]

def rescale_polygon_list(polygon_list, scale, conv=lambda x: x):
	return [ rescale_polygon(polygon, scale, conv) for polygon in polygon_list ]


def transform_point(p, scale, translate, flipX, flipY, conv):
	x, y = p
	dx, dy = translate
	scaleX, scaleY = scale, scale
	if flipX:
		scaleX = -scaleX
	if flipY:
		scaleY = -scaleY
	return ( conv(dx+(x*scaleX)), conv(dy+(y*scaleY)) ) 

def transform_polygon(polygon, scale, translate, flipX, flipY, conv):
	return [ transform_point(p, scale, translate, flipX, flipY, conv) for p in polygon ]

def transform_polygon_list(polygon_list, scale, translate=(0,0), flipX=False, flipY=False, conv=lambda x: x):
	return [ transform_polygon(polygon, scale, translate, flipX, flipY, conv) for polygon in polygon_list ]

def translate_polygon(polygon_list, translate):
	dx, dy = translate
	return [ (x+dx, y+dy) for x, y in polygon ]

def translate_polygon_list(polygon_list, translate):
	return [ translate_polygon(polygon, translate) for polygon in polygon_list ]

