use <eggshell.scad>
use <switch.scad>
use <driver.scad>

SWITCH_POS = [0,-8.3,0];
DRIVER_POS = [0,3.,0];

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
		translate([0,0,5])
		{
		    intersection()
		    {
				cube( size = [30, 30, 10], center=true );
				difference()
				{
					eggshell();
					translate ([0,12.5,0])
						rotate([90,0,0])
							cylinder(r = .5, h=6, $fn=20, center=true);
				}
			}
		}
		cylinder(r = 11.5, h=1.2);
		translate (DRIVER_POS)
			rotate ([0,0,90])
				driver_positive(1); 
	}
	cutouts();
}

render();
