
module switch_cutout()
{
	translate ([ 0, 0, .69 ])
		union ()
		{
			cube(size=[6.9,3,5], center=true);
			translate ([0,-2.4,0])
				cube(size=[4,5,1.42], center=true);
		}
}

switch_cutout();

