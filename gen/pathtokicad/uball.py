#!/usr/bin/python

from pathtokicad import *
import svgpath

in_dpi, out_dpi = 25.4, 10000.
svgpath.scale = out_dpi/in_dpi
svgpath.new_scale = 1000


fill_paths = [

	(FRONT_MASK,   "uball/mask_top.path"), # soldermask front
	(FRONT_COPPER, "uball/copper_top.path"),
	(BACK_COPPER,  "uball/copper_bottom.path"),
	(BACK_MASK,    "uball/mask_bottom.path"),
#	(FRONT_PASTE,  "uball/paste.path"),
]

segment_paths = [
]

pads = [
]

print_module("uball", fill_paths, segment_paths, pads)
 
