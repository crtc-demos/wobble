// Persistence Of Vision raytracer version 3.0 sample file.
// Use copies of this file for starting your own scenes.

#version 3.5;

global_settings { assumed_gamma 2.2 }

#include "colors.inc"
#include "textures.inc"
#include "stars.inc"
#include "skies.inc"
#include "shapes.inc"
#include "stones.inc"
#include "rand.inc"

camera
{
#switch (clock)
  /* Parabolic environment map.  */
  #case (1)
	orthographic
	location  <0.0, 0.5, -2.0>
        look_at   <0.0, 0.5, 0.0>
	up        y*2
	right     x*2
  #break
  #case (2)
	orthographic
        location  <0.0, 0.5, 2.0>
        look_at   <0.0, 0.5, 0.0>
	up        -y*2
	right     -x*2
  #break
  
  /* Sky cube.  */
  #case (3)
	location <0, 0, 0>
	look_at	+x
	up +y
	right +z
	angle 90
	translate <0, 0.5, 0>
  #break
  #case (4)
	location <0, 0, 0>
	look_at	-x
	up -y
	right +z
	angle 90
	translate <0, 0.5, 0>
  #break
  #case (5)
	location <0, 0, 0>
	look_at	-y
	up +x
	right +z
	angle 90
	translate <0, 0.5, 0>
  #break
  #case (6)
	location <0, 0, 0>
	look_at	+y
	up -x
	right +z
	angle 90
	translate <0, 0.5, 0>
  #break
  #case (7)
	location <0, 0, 0>
	look_at	+z
	up +x
	right +y
	angle 90
	translate <0, 0.5, 0>
  #break
  #case (8)
	location <0, 0, 0>
	look_at	-z
	up -x
	right -y
	angle 90
	translate <0, 0.5, 0>
  #break
#end
}

/* Paraboloid_Z, etc. is defined as:

   x^2 + y^2 - z = 0
   
   i.e.,
   
   x^2 + y^2 = z

   dual-paraboloid environment mapping wants a paraboloid like so:
   
   z = 0.5 - 0.5*(x^2 + y^2)
   
   (with x^2 + y^2 <= 1)
   
   so that the edges of the image correspond to gradient -1/1 & z=0.
   
   ignore the constant offset 0.5 (turns into translate), then:
   
   z = - (x^2 + y^2)
*/

#switch (clock)

  #case (1)

intersection
{
  object
  {
    Paraboloid_Z

    scale <1, 1, 0.5>
    translate <0, 0.5, -0.5>
  }
 
  /*sphere
  {
    <0, 0.5, 0>, 1
  }*/

  no_shadow

  finish
  {
    reflection 1.0
  }
}
  
  #break
  
  #case (2)

object
{
  Paraboloid_Z
  
  scale <1, 1, -0.5>
  translate <0, 0.5, 0.5>
  
  no_shadow
  
  finish
  {
    reflection 1.0
  }
}

  #break
#end

/*sphere {
  <5, 0.5, 0>, 1
  
  pigment
  {
    color Yellow
  }
  
  finish
  {
    diffuse 0.5
    specular 1.0
  }
}*/

sky_sphere { S_Cloud1 }

light_source {
  <-20, 50, 50> color White
}

#declare Lz = -40;
#while(Lz < 40)

  #declare Lx = -40;
  #while(Lx < 40)
    box {
      <Lx, -5, Lz>,
      <Lx+1, RRand(-10, -5 + abs(Lx/5) + abs(Lz/5), 0), Lz+1>
      texture {
	T_Stone29
	scale 4
      }
    }
  #declare Lx = Lx + 1;
  #end

#declare Lz = Lz + 1;
#end
