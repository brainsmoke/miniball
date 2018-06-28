use <eggshell20mm.scad>
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
		//translate (DRIVER_POS)
			//rotate([0,0,90])
				//driver_cutout(); 
                translate([0,0,-.1]) cylinder(r = 9, h=10, $fn=100);
	}
}

	difference()
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
		cutouts();
	}


render();
