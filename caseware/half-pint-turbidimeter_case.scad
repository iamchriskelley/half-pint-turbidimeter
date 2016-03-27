use_led0multi = true;
use_led1 = false;
use_led2 = true;
use_led3 = true;
led1_rotz = 32;
led2_rotz = -16;
led3_rotz = 16;
led_holder_x = 11;
led_holder_y = 10;
led_holder_z = 74;
led1_place = [15,-18,led_holder_z/2];
led2_place = [48,-12,led_holder_z/2];
led3_place = [48,-2,led_holder_z/2];
led1_zoffset = [0,0,24];
led2_zoffset = [0,0,24];
led3_zoffset = [0,0,24];
$fn = 50;
scl = 1.0;
ovalr0 = 34;
ovalr1 = 25;

scale([scl,scl,scl])
difference(){
	union(){
		//translate([0,-4,0]) cube([71,22,24]);//test_case();
        translate([5,-4,22]) cube([60,20,2]);
        translate([68/2 + 1.5,-3,22]) oval(ovalr0,ovalr1,4);
		translate([5,5,24]) cube([60,2,50]);
		translate([5,5,24]) cube([2,10,50]);
		translate([5,13,24]) cube([5,2,50]);
		translate([63,5,24]) cube([2,10,50]);
		translate([60,13,24]) cube([5,2,50]);
		translate([10,13,24]) cube([50,2,5]);
        translate([30.5,-8,0]) cylinder(r1=14.6,r2=14.6,h=81.2);
        translate([6,-13,0]) cube([led_holder_x,led_holder_y,led_holder_z]);
        //translate([60,-5.2,0]) cylinder(r=11.2,h=70);
        if(use_led1){translate(led1_place) rotate([0,0,led1_rotz]) cube([led_holder_x,led_holder_y,led_holder_z], center=true);}
        if(use_led2){translate(led2_place) rotate([0,0,led2_rotz]) cube([led_holder_x,led_holder_y,led_holder_z], center=true);}
        if(use_led3){translate(led3_place) rotate([0,0,led3_rotz]) cube([led_holder_x,led_holder_y,led_holder_z], center=true);}
	}
	//sensor viewport
    translate([27,2,56.5])cube([7,8,6.5]);

	//cuvette 
	translate([30.5,-8,41]) cylinder(r=12.3, h=11);
    translate([30.5,-8,51]) cylinder(r=12.3, h=28.4);
    translate([30.5,-8,78.4]) cylinder(r=13.8, h=5);
    
    //led wires
    translate([12,5,60]) cube([3,5,5]);

    //led hole //z=59 for one led alone
    if(use_led0multi){
        translate([10,-8,64]) rotate([0,90,0])  cylinder(r=2.5, h=20, center=true);
        translate([10,-8,57.6]) rotate([0,90,0])  cylinder(r=2.5, h=20, center=true);
        translate([10,-8,51.2]) rotate([0,90,0])  cylinder(r=2.5, h=20, center=true);
    }else{
        translate([10,-8,60]) rotate([0,90,0])  cylinder(r=2.5, h=20, center=true);
    }
    if(use_led1){translate(led1_place + led1_zoffset) rotate([0,90,led1_rotz])  cylinder(r=2.5, h=20, center=true);}
    if(use_led2){
        translate(led2_place + led2_zoffset) rotate([0,90,led2_rotz])  cylinder(r=2.5, h=20, center=true);
    }
    if(use_led3){translate(led3_place + led3_zoffset) rotate([0,90,led3_rotz])  cylinder(r=2.5, h=20, center=true);}

    /*//battery port
	translate([3,7,15]) cube([69,5,7]);
	rotate([0,90,0]) translate([-11,7,3]) cylinder(r=10, h=69);
	translate([60,7.5,24]) cube([25,5.5,12]);
    translate([60,-5.2,24]) cylinder(r=9.2,h=70);*/
    
    translate(led2_place + [-2,-6,led_holder_z/2-4]) rotate([0,0,led2_rotz]) cube([3,led_holder_y+2,4]);
    translate([10,-13,led_holder_z - 4]) cube([3,led_holder_y,4]);
    
    translate([0,-50,-2]) cube([100,100,24]);  
    //translate([45,-50,-2]) cube([100,100,100]);
    //translate([0,0,0]) cube([15,100,100]);
    //translate([0,-50,0]) cube([100,100,40]);
}


module oval(w,h, height, center = true) { 
    scale([1, h/w, 1]) cylinder(h=height, r=w, center=center); 
} 

module build_envelope(){
    h = 55;
    ishell = 0.8;
    oshell = 2.4;
    color([0,0,1,0.5])
    scale([scl,scl,scl])
    //[30.5,-8,0]
    
    //[5,13,24]
    //35.2,-3,0
    difference(){
        translate([ovalr0 + oshell/2,-3,22+h/2]) oval(ovalr0 + oshell, ovalr1 + oshell, h);
        translate([ovalr0 + oshell/2,-3,22+h/2]) oval(ovalr0 + ishell, ovalr1 + ishell, h);
    }
}

//build_envelope();