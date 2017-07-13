#!/usr/bin/python

import svgpath

ROUTE_SPEED=100
ROUTE_Z_SPEED=20

BASE=0.

ROUTE_MARGIN=0.2
MOVE_MARGIN=2.

ROUTE_TOP_Z=BASE+ROUTE_MARGIN

MOVE_Z=BASE+MOVE_MARGIN

ROUTE_Z=BASE-2

END_Z=40

def up():
	print "G0 Z%f" % (MOVE_Z,)

def down(z):
	print "G0 Z%f" % (ROUTE_TOP_Z,)
	print "G1 F%f Z%f" % (ROUTE_Z_SPEED, z)

def move_unsafe(x, y, z):
	print "G0 X%f Y%f Z%f" % (x, y, z)

def move_unsafe_xy(x, y):
	print "G0 X%f Y%f" % (x, y)

def route(x, y, z, speed=ROUTE_SPEED):
	print "G1 F%f X%f Y%f Z%f" % (speed, x, y, z)

def move(x, y, z):
	up()
	move_unsafe_xy(x, y)
	down(z)

def end():
	 print "G0 Z%f" % (END_Z,)

def route_path(path, z, speed):
	if len(path) == 0:
		return

	x, y = path[0]
	move(x, y, z)
	for coord in path[1:]:
		x, y = coord
		route(x, y, z, speed)

def fill_square(x, y, w, h, milldiameter, overlapratio=.6):
	millradius = milldiameter/2.
	d=milldiameter*(1-overlapratio)
	x1, x2 = x+millradius, x+w-millradius
	y1, y2 = y+millradius, y+h-millradius
	poly = []
	if x1 > x2 or y1 > y2:
		return poly

	poly.append( (x1, y1) )

	while True:
		poly.append( (x2, y1) )
		y1 += d
		if y1 >= y2:
			break

		poly.append( (x2, y2) )
		x2 -= d
		if x1 >= x2:
			break
		
		poly.append( (x1, y2) )
		y2 -= d
		if y1 >= y2:
			break
		
		poly.append( (x1, y1) )
		x1 += d
		if x1 >= x2:
			break

	return poly

def voodoo_spell():
	"""prologue: I have no clue what this does"""
	print "G17"
	print "G90"
	print "G91.1"
	print "G21"
	print "G17 G64 P0.0001 M3 S3000"

def kthxbye():
	"""epilogue: I have no clue what this does"""
	print "M5"
	print "M2"


def readall(f):
	d=r=f.read()
	while True:
		r=f.read()
		if r == '':
			return d
		d=d+r	

def load_svg_path(filename, scale=25.4/90., translate=(0,0), flipX=False, flipY=False):
	with open(filename) as f:
		polygons = svgpath.path_to_polygons(readall(f))
		polygons = svgpath.transform_polygon_list(polygons, scale=scale, translate=translate, flipX=flipX, flipY=flipY, conv=float)
	return polygons

def print_gcode_start():
	voodoo_spell()

def print_gcode_end():
	end()
	kthxbye()

def print_gcode_from_svg(paths, scale=25.4/90., translate=(0,0), flipX=False, flipY=False):

	for depth, speed, filename in paths:
		polygons = load_svg_path(filename, scale, translate, flipX, flipY)
		for p in polygons:
			route_path(p, depth, speed)

