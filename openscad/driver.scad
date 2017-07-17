
function snub(xmin, ymin, xmax, ymax, d) = [ [xmax, ymax-d], [xmax, ymin+d], [xmax-d, ymin], [xmin+d, ymin], [xmin, ymin+d], [xmin, ymax-d], [xmin+d, ymax], [xmax-d, ymax] ];

BOTTOM_THICKNESS = .8;
D = .01;

//DRIVER_PCB_SHAPE = [ [6,4], [4,6], [-4,6], [-6,4], [-6, -4], [-4, -6], [4, -6], [6, -4] ];
DRIVER_KEEPOUT_SHAPE = snub(-6.2, -6.2, 6.2, 6.2, 2);
DRIVER_WALL_SHAPE = snub(-7.2, -7.2, 7.2, 7.2, 2);


DRIVER_PCB_SHAPE = snub(-6, -6, 6, 6, 2);
DRIVER_CLAMP_SHAPE = [ [-5.4-D, 5], [5.4+D, 5], [4.2+D, 6.2+D], [-4.2-D, 6.2+D] ];
DRIVER_ISP_KEEPOUT = snub(-5, -.5, 3, 4.5, .5);//		-3, -4.5, 5, .5, .5);
DRIVER_STANDOFF_1 = [ [-1.7, -6.2-D], [-1.7, -4.5], [-2.8, -4.5], [-2.8, -6.2-D] ] ;
DRIVER_STANDOFF_2 = [ [ 1.7, -6.2-D], [ 1.7, -4.5], [ 2.8, -4.5], [ 2.8, -6.2-D] ] ;


module driver_cutout()
{
	difference()
	{
		union()
		{	
			translate([0,0,BOTTOM_THICKNESS])
				linear_extrude(5)
					polygon(DRIVER_KEEPOUT_SHAPE);
			translate([0,0,-D])
				linear_extrude(5)
					polygon(DRIVER_ISP_KEEPOUT);
			driver_isp_hole();
		}
		driver_standoffs();
	}
}

module driver_standoffs()
{
	translate([0,0,BOTTOM_THICKNESS-D])
		linear_extrude(2+D)
		{
				polygon(DRIVER_CLAMP_SHAPE);
				polygon(DRIVER_STANDOFF_1);
				polygon(DRIVER_STANDOFF_2);
		}
}

module driver_wall(w)
{
	difference ()
	{
		translate([0,0,BOTTOM_THICKNESS-D])
			linear_extrude(3+D)
				polygon(snub(-6.2-w, -6.2-w, 6.2+w, 6.2+w, 2));
		translate([0,0,BOTTOM_THICKNESS-D-D])
			linear_extrude(3+D+D+D)
				polygon([ [-4.2, -5], [4.2, -5], [4.2, -8], [-4.2, -8] ]);
	}
}

module driver_isp_hole()
{
	translate([0,0,-100])
		linear_extrude(105)
			polygon(DRIVER_ISP_KEEPOUT);
}


module driver_positive(w)
{
	driver_wall(w);
	driver_standoffs();
}

difference()
{
	union()
	{
		cylinder(r=12.5, h=1.2);
		driver_positive(1);
	}
	driver_cutout();
}
