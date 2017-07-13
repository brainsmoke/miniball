#!/usr/bin/python

import sys, math, itertools

import svgpath

FRONT_MASK   = "23"
FRONT_SILK   = "21"
FRONT_PASTE  = "19"
FRONT_COPPER = "15"

BACK_MASK    = "22"
BACK_SILK    = "20"
BACK_COPPER  = "0"
BACK_PASTE   = "18"

EDGES        = "28"

cubic_sections = 32

def roundint(x):
	return int(round(x))

def dist(a, b):
	ax, ay = a
	bx, by = b
	return math.sqrt((ax-bx)**2 + (ay-by)**2)

def cross_product(a, b, c):
	ax, ay = a
	bx, by = b
	cx, cy = c
	return (bx-ax) * (cy-ay) - (by-ay) * (cx-ax)

def counter_clockwise(a, b, c):
	z = cross_product(a, b, c)
	if z > 0:
		return 1
	elif z < 0:
		return -1
	else:
		return 0

def intersect(l1, l2):
	a, b = l1
	c, d = l2
	return ( counter_clockwise(a,c,d) != counter_clockwise(b,c,d) and 
	         counter_clockwise(a,b,c) != counter_clockwise(a,b,d) )

def interpolate(pos1, pos2, d):
	x1, y1 = pos1
	x2, y2 = pos2
	return ( x1*(1-d) + x2*d, y1*(1-d) + y2*d )

def vector_add(a, b):
	return tuple( i+j for i, j in zip(a,b) )

def vector_sub(a, b):
	return tuple( i-j for i, j in zip(a,b) )

#def cubic_spline( start, guide1, guide2, end ):
#	n = min(int(dist(start, end)*scale/40.)+1, cubic_sections)
#
#	v = []
#	for i in xrange(1, n+1):
#		d = i/float(n)
#		a = interpolate(start, guide1, d)
#		b = interpolate(guide1, guide2, d)
#		c = interpolate(guide2, end, d)
#
#		ab = interpolate(a, b, d)
#		bc = interpolate(b, c, d)
#		abc = interpolate(ab, bc, d)
#		v.append(abc)
#	return v

def get_coords(s):
	return map(float, s)

def quadrant(p):
	px,py = p
	if py < 0:
		return int(px>0)
	else:
		return 3-int(px>0)

def polygon_rotations(poly, center):
	r = 0
	x, y = vector_sub(poly[0], center)
	q = quadrant( (x, y) )
	for p in poly[1:]:
		lastx, lasty = x, y
		lastq = q

		x, y = vector_sub(p, center)
		q = quadrant( (x, y) )

		if x|y == 0:
			return 0
		elif q == lastq:
			pass
		elif q == (lastq+1)%4:
			r += 1
		elif q == (lastq+3)%4:
			r -= 1
		elif (lastx*y < x*lasty) == (y*lasty > 0):
			r += 2
		else:
			r -= 2
	if not -1 <= r % 4 <= 1:
		print >> sys.stderr , "MEH r="+str(r), [poly]
		sys.exit(1)
	return r//4

def polygon_crossproduct_sum(poly):
	total = 0
	a = poly[0]
	for i in range(len(poly)-2):
		b,c = poly[i+1:i+3]
		total += cross_product(a, b, c)
	return total

def remove_subsubsets(subsets):
	sets = subsets.keys()
	for s_a in sets:
		for s_b in sets:
			for s_c in sets:
				if s_a in subsets[s_b] and s_b in subsets[s_c] and s_a in subsets[s_c]:
					subsets[s_c].remove(s_a)

def nest_depth(parentset, key):
	if parentset[key] == []:
		return 0
	else:
		return 1+nest_depth(parentset, parentset[key][0])

def get_cutout_mapping(polygon_list):
	parents = dict( (n,[]) for n in xrange(len(polygon_list)) )
	children = dict( (n,[]) for n in xrange(len(polygon_list)) )
	bboxes = [ ( min(x for x,y in p),
	             min(y for x,y in p),
	             max(x for x,y in p),
	             max(y for x,y in p) ) for p in polygon_list ]

	for a, p_a, in enumerate(polygon_list):
		for b, p_b, in enumerate(polygon_list):
			if a != b:
				min_ax, min_ay, max_ax, max_ay = bboxes[a]
				min_bx, min_by, max_bx, max_by = bboxes[b]
				if ( min_bx <= min_ax <= max_ax <= max_bx and
				     min_by <= min_ay <= max_ay <= max_by ):

					r = polygon_rotations(p_b, p_a[0])
					if r == 1:
						print >> sys.stderr, "orientation weird"
						sys.exit(1)
					if r != 0:
						parents[a].append(b)
						children[b].append(a)

	remove_subsubsets(parents)
	remove_subsubsets(children)

	for p in children.keys():
		if nest_depth(parents, p) & 1 == 1:
			del children[p]

	return children

def close_polygons(polygons):
	for polygon in polygons:
		if tuple(polygon[0]) != tuple(polygon[-1]):
			polygon.append(polygon[0])
	return polygons

def counter_clockwise(polygons):
	for polygon in polygons:
		if polygon_crossproduct_sum(polygon) > 0:
			polygon.reverse()
	return polygons

def weakly_simplefy_polygon(polygon, cutouts):
	for c in cutouts:
		c.reverse()
	while len(cutouts) > 0:
		distlist = [ (dist(pp, cp), pn, cn, c) for pn,pp in enumerate(polygon[:-1]) for c in cutouts for cn,cp in enumerate(c[:-1]) ]
		distlist.sort(cmp=lambda a, b: int(a[0]-b[0]))
		print >> sys.stderr, len(distlist)
		for _, pn, cn, c in distlist:
			line1 = (polygon[pn], c[cn])

			for line2 in itertools.chain( zip(p[:-1], p[1:]) for p in [polygon]+cutouts ):
				if polygon[pn] not in line1 and intersect(line1, line2):
					break
			else:
				print >> sys.stderr, polygon[pn], c[cn], _
				polygon[pn:pn] = [polygon[pn]] + c[cn:-1] + c[:cn+1] 
				cutouts.remove(c)
				break
	return polygon

def weakly_simplefy(polygons):
	polygons = close_polygons(polygons)
	polygons = counter_clockwise(polygons)
	# slow
	mapping = get_cutout_mapping(polygons)
	mapping_keys = mapping.keys()
	mapping_keys.sort()
	print >> sys.stderr, mapping
	# slow as hell
	return [ weakly_simplefy_polygon(polygons[num], [polygons[x] for x in mapping[num]]) for num in mapping_keys ]

#
#def path_to_polygons(data):
#
#	values = (x for x in data.replace(',', ' ').replace('\n',' ').split(' ') if x != '' )
#
#	mode = 'z'
#	pos = (0.,0.)
#
#	polygons = []
#	p = []
#
#	for x in values:
#
#		if x in 'zZ':
#			pos = p[0]
#			p.append( pos )
#
#		if x in 'zZmM':
#			if len(p) > 0:
#				polygons.append( p )
#				p = []
#
#		if x in 'zZmclMCL':
#			mode = x
#			continue
#
#		if mode == 'm':
#			mode = 'l'
#
#		if mode == 'M':
#			mode = 'L'
#
#		if mode == 'l':
#			pos = vector_add(pos, get_coords((x, values.next())))
#			p.append( pos )
#
#		elif mode == 'L':
#			pos = get_coords((x, values.next()))
#			p.append( pos )
#
#		elif mode in 'cC':
#			start  = pos
#			guide1 = get_coords( (x, values.next()) )
#			guide2 = get_coords( (values.next(), values.next()) )
#			end    = get_coords( (values.next(), values.next()) )
#
#			if mode == 'c':
#				guide1 = vector_add(pos, guide1)
#				guide2 = vector_add(pos, guide2)
#				end    = vector_add(pos, end)
#
#			pos = end
#			p.extend( cubic_spline(start, guide1, guide2, end) )
#		else:
#			print "ERROR: " + x
#			sys.exit(1)
#
#	if len(p) > 0:
#		polygons.append( p )
#
#	return polygons

def coord_fmt( coords ):
	x, y = coords
	return "%d %d" % (x, y)

def coord_fmt_new( coords ):
	x, y = coords
	return "(xy %.3f %.3f) " % (x/1000., y/1000.)

#def rescale_point(p, scale, conv=lambda x: x):
#	x, y = p
#	return ( conv(x*scale), conv(y*scale) )
#
#def rescale_polygon(polygon, scale, conv=lambda x: x):
#	return [ rescale_point(p, scale, conv) for p in polygon ]
#
#def rescale_polygon_list(polygon_list, scale, conv=lambda x: x):
#	return [ rescale_polygon(polygon, scale, conv) for polygon in polygon_list ]

def pad_grid(coords, w, h, pitch):
	x, y = coords
	return [ (x+pitch*i, y+pitch*j) for i in xrange(w) for j in xrange(h) ]

def print_pad(coords, size):
	print """$PAD
Sh "1" C """+str(int(size)+200)+""" """+str(int(size)+200)+""" 0 0 0
Dr """+str(int(size))+""" 0 0
At STD N 00E0FFFF
Ne 0 ""
Po """+coord_fmt(coords)+"""
$EndPAD"""

def print_polygon(polygon, layer):
	print 'DP 0 0 0 0 %d 1 %s' % (len(polygon), layer)
	for point in polygon:
		print "Dl " + coord_fmt(point)

def print_segments(polygon, layer, width):
	for from_, to in zip(polygon[:-1], polygon[1:]):
		print "DS %s %s %d %s" % (coord_fmt(from_), coord_fmt(to),width,layer)

def print_zone(polygon, layer, label):
	print ' 0\n'.join("ZCorner " + coord_fmt(point) for point in polygon) + ' 1'

def print_zone_new(polygon):
	print '\n'.join(coord_fmt_new(point) for point in polygon)

def print_module(name, fill_paths, segment_paths, pads):

	print """PCBNEW-LibModule-V1
$INDEX
"""

	print name
	print """$EndINDEX
$MODULE """ + name + """
Po 0 0 0 15 00000000 00000000 ~~
Li """ + name

	for layer, filename in fill_paths:
		print >> sys.stderr , filename
		with open(filename) as f:
			polygons = svgpath.path_to_polygons(f.read(1000000))

		polygons = svgpath.rescale_polygon_list(polygons, svgpath.scale, roundint)
		polygons = weakly_simplefy(polygons)

		for p in polygons:
			print_polygon(p, layer)

	for layer, filename, width in segment_paths:
		with open(filename) as f:
			polygons = svgpath.path_to_polygons(f.read(1000000))

		polygons = svgpath.rescale_polygon_list(polygons, svgpath.scale, roundint)

		for p in polygons:
			print_segments(p, layer, width*svgpath.scale)

	for topleft, w, h, pitch, size in pads:
		pads = pad_grid(topleft, w, h, pitch)
		for pad in pads:
			print_pad(svgpath.rescale_point(pad, svgpath.scale, roundint), size)

	print """$EndMODULE """ + name + """
$EndLIBRARY"""

def print_zones(zone_paths):

	for layer, label, filename in zone_paths:
		print """$CZONE_OUTLINE
ZInfo 525A79DA 1 """+'"'+label+'"'+"""
ZLayer """+layer+"""
ZAux 4 E
ZClearance 200 T
ZMinThickness 100
ZOptions 0 16 F 200 200
ZSmoothing 0 0"""
		with open(filename) as f:
			polygons = svgpath.path_to_polygons(f.read(1000000))

		polygons = svgpath.rescale_polygon_list(polygons, svgpath.scale, roundint)
		polygons = weakly_simplefy(polygons)

		for p in polygons:
			print_zone(p, layer, label)
		print """$endCZONE_OUTLINE"""

def print_zones_new(zone_paths):

	for layer, label, filename in zone_paths:
		with open(filename) as f:
			polygons = svgpath.path_to_polygons(f.read(1000000))

		polygons = svgpath.rescale_polygon_list(polygons, svgpath.new_scale, roundint)
		polygons = weakly_simplefy(polygons)

		for p in polygons:
			print_zone_new(p)

