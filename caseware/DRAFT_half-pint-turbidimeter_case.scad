//GLOBALS
$fn = 50;
scl = 1.0;
scale([scl,scl,scl]) turbidimeter_case();
//--------------------------------------
//READABLE VARIABLES
cuvette_holder_z = 50;
cuvette_holder_radius = 14.6;

cuvette_chamber_z = 28.4;
cuvette_chamber_radius = 12.3;

cuvette_lip_z = 4;
cuvette_lip_radius = 13.8;

led_mount_x = 11;
led_mount_y = 10;
led_mount_z = 45.2;
led_mount_notch_x = 2.8;
led_mount_notch_z = 1.6;

//ABBREVIATED HELPER VARIABLES
chz = cuvette_holder_z;
ch_r = cuvette_holder_radius;
ccz = cuvette_chamber_z;
ccr = cuvette_chamber_radius;
clz = cuvette_lip_z;
clr = cuvette_lip_radius;
lmx = led_mount_x;
lmy = led_mount_y;
lmz = led_mount_z;
lmnx = led_mount_notch_x;
lmnz = led_mount_notch_z;
//--------------------------------------
//MODULES
module turbidimeter_case(){
    cuvette_holder();
    case_base();
}

module case_base(){
    difference(){
        translate([0,0,1])oval(32,25,2);
        translate([-50,-38,0])cube([100,20,3]);
    }
}

//INDIVIDUAL PARTS
module cuvette_holder(){
    difference(){
        union(){
            cylinder(r=ch_r,h=chz);
            translate([ch_r-2,-lmy/2,0]) led_mount();
            translate([-ch_r+2,lmy/2,0]) rotate([0,0,180]) led_mount();
            translate([-ch_r+9,lmy/2+8,0]) rotate([0,0,135]) led_mount();
            //translate([lmy/2,ch_r-2,0]) rotate([0,0,90]) led_mount();
            //translate([-ch_r+4,lmy/2+4,0]) rotate([0,0,162]) led_mount();
            //translate([-ch_r+1,lmy/2-4,0]) rotate([0,0,198]) led_mount();
            translate([-25,-ch_r,0]) pcb_mount();
        }
        translate([0,0,chz-ccz]) cylinder(r=ccr, h=ccz);
        translate([0,0,chz-clz]) cylinder(r=clr, h=clz);
    }
}

module pcb_mount(){
    TPZO = 8;  //temp pcb z-offset
    difference(){
        cube([50,2,lmz]);
        translate([10,0,6]) rotate([90,0,0]) cylinder(r=1.2,h=4, center=true);
        translate([10,0,20]) rotate([90,0,0]) cylinder(r=1.2,h=4, center=true);
        
        translate([3,0,31+TPZO]) rotate([90,0,0]) cylinder(r=1.2,h=4, center=true);
        translate([6,0,22+TPZO]) rotate([90,0,0]) cylinder(r=1.2,h=4, center=true);
        translate([50-3,0,31+TPZO]) rotate([90,0,0]) cylinder(r=1.2,h=4, center=true);
        translate([50-6,0,22+TPZO]) rotate([90,0,0]) cylinder(r=1.2,h=4, center=true);
        
        translate([21.5,0,18.25+TPZO]) cube([9,2,10]);
    }
}

module led_mount(){
    difference(){
        cube([lmx,lmy,lmz]);
        translate([12.2,lmy/2-4,20]) rotate([0,0,90]) led_backpack();
        translate([lmx/2-lmnx/2,0,lmz-lmnz]) cube([lmnx,lmy,lmnz]);
    }
}

module led_backpack(){
    cube([8,2,21]);
    translate([0,2,4]) cube([8,2,4]);
    rotate([90,0,0]) translate([4,13,-2.5]) cylinder(r = 3.5,h = 1, center = true);
    rotate([90,0,0]) translate([4,13,-8]) cylinder(r = 3, h = 10,center = true);
    rotate([90,0,0]) translate([4,19,-6]) cylinder(r = 1.5,h = 8,center = true);
    rotate([90,0,0]) translate([4,2,-6]) cylinder(r = 1.5,h = 8,center = true);
}

module oval(w,h, height, center = true) { 
    scale([1, h/w, 1]) cylinder(h=height, r=w, center=center); 
} 
