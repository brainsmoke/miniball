#!/usr/bin/python

from pathtogcode import *

DRILL_DEPTH=-1

centers_first = [ (26.323, 60.337), (39.710, 66.119), (14.786, 74.821), (29.288, 76.318), (22.421, 89.185), (38.579, 87.555) ]
copies = [ (26.323, 60.337), (71.482, 69.340), (26.323, 11.796), (71.482, 11.796) ]

offx, offy = copies[0]
offsets = [ (x-offx, y-offy) for x, y in copies ]
holes = [ (dx+x, dy+y) for dx,dy in offsets for x, y in centers_first ]

# translate from inkscape to svg coords
holes = [ (x, 100-y) for x, y in holes ]

print_gcode_start()
#print_gcode_from_svg(paths)

for x, y in holes:
	move(x, y, DRILL_DEPTH)

print_gcode_end()
 
