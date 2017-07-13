#!/usr/bin/python

from pathtogcode import *

paths = [

#	depth speed filename
	(-2, 500, "microball_jig/zigzag.path"),
	(-2, 100, "microball_jig/circles.path"),
	(-2, 500, "microball_jig/outline.path"),
]

print_gcode_start()
#print_gcode_from_svg(paths)
#route_path( fill_square(0,0,100,100,6), -2, 500 )
#move(0, 0, -2)
#move(100, 0, -2)
#move(100, 100, -2)
#move(0, 100, -2)

move(5, 5, -4)
move(95, 0, -4)
move(95, 95, -4)
move(0, 95, -4)

print_gcode_end()
 
