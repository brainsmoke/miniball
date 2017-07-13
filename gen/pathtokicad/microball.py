#!/usr/bin/python

from pathtokicad import *

fill_paths = [

	(FRONT_MASK,   "microball/mask_top.path"), # soldermask front
	(FRONT_COPPER, "microball/copper_top.path"),
	(BACK_COPPER,  "microball/copper_bottom.path"),
	(BACK_MASK,    "microball/mask_bottom.path"),
	(FRONT_PASTE,  "microball/paste.path"),
]

segment_paths = [
]

pads = [
]

print_module("microball", fill_paths, segment_paths, pads)
 
