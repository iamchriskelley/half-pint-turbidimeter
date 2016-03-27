$fn = 50;
scl = 1.0;
ovalr0 = 34;
ovalr1 = 25;

module oval(w,h, height, center = true) { 
    scale([1, h/w, 1]) cylinder(h=height, r=w, center=center); 
} 

module build_envelope(){
	 gasket_thickness = 0;
    h = 54 + gasket_thickness;
    sqw = 65;
    sqd = 9;
    sqy = 12;
    ishell = 0.8;
    oshell = 2;
    //color([0,0,1,0.5])
    scale([scl,scl,scl])
    difference(){
    union(){
        difference(){
            union(){
                
                translate([0,0,h/2]) oval(ovalr0 + oshell, ovalr1 + oshell, h);
                translate([-sqw/2,sqd,0]) cube([sqw,20,h]);
            }
            translate([0,0,h/2+oshell]) oval(ovalr0 + ishell, ovalr1 + ishell, h);
            translate([4.7,-5,0]) cylinder(r=15,h=3);
            translate([-sqw/2+oshell,sqy,oshell]) cube([sqw-2*oshell,sqd,h]);
            translate([-sqw/2,sqy+sqd,0]) cube([sqw,100,h]);
        }
        translate([-sqw/2+oshell,sqy+sqd-oshell,0]) cube([6,oshell,h]);
        translate([-sqw/2,sqy+sqd-oshell,0]) cube([21,oshell,h/2+4]);
        translate([sqw/2-10,sqy+sqd-oshell,0]) cube([9,oshell,h]);
        translate([sqw/2-15,sqy+sqd-oshell,0]) cube([14,oshell,h/2+4]);
        translate([-sqw/2,sqy+sqd-oshell,0]) cube([60,oshell,5]);
    }
        //translate([-sqw/2+5,sqy+sqd-oshell,4]) cube([10,oshell,15]);
    }
}

build_envelope();