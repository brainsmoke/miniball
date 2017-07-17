use <eggshell2.scad>
use <switch.scad>
use <driver.scad>

SWITCH_POS = [0,-6.81,0];
DRIVER_POS = [0,1.4,0];

module cutouts()
{
	union ()
	{
		translate(SWITCH_POS)
			switch_cutout();
		translate (DRIVER_POS)
			rotate([0,0,90])
				driver_cutout(); 
	}
}

difference()
{
	union()
	{
		translate([0,0,4])
		{
			intersection()
			{
				cube( size = [30, 30, 8], center=true );
				difference()
				{
					eggshell20mm();
					translate ([0,10,0])
						rotate([90,0,0])
							cylinder(r = .5, h=6, $fn=20, center=true);
				}
			}
		}
		cylinder(r = 9.1, h=1.2);
		difference()
		{    
			translate (DRIVER_POS)
				rotate([0,0,90])
					driver_positive(.5); 
			translate([0,0,4])
				rotate([180,0,0])
					eggshell20mm();
		}
	}
	cutouts();
}


render();
