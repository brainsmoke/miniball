#!/usr/bin/python

from pathtogcode import *

DRILL_DEPTH=-1

paths = [

#       depth speed filename
	(-1, 500, "microball/cutting-.9mm.path"),
]

print_gcode_start()
print_gcode_from_svg(paths)
print_gcode_end()
 
