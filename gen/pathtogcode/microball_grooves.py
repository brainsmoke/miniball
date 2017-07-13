#!/usr/bin/python

from pathtogcode import *

GROOVE_DEPTH=-.6

paths = [

#       depth speed filename
	(GROOVE_DEPTH, 500, "microball/grooves.path"),
]

print_gcode_start()
print_gcode_from_svg(paths)
print_gcode_end()
 
