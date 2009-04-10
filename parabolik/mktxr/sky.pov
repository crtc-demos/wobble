// Persistence Of Vision raytracer version 3.0 sample file.
// Use copies of this file for starting your own scenes.

#version 3.5;

global_settings { assumed_gamma 2.2 }

#include "colors.inc"
#include "textures.inc"
#include "stars.inc"
#include "skies.inc"
#include "shapes.inc"

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

/*
union
{
  cylinder { <-3,  3,  3>, < 3,  3,  3>, 0.2 }
  cylinder { <-3, -3,  3>, < 3, -3,  3>, 0.2 }
  cylinder { <-3,  3, -3>, < 3,  3, -3>, 0.2 }
  cylinder { <-3, -3, -3>, < 3, -3, -3>, 0.2 }

  cylinder { < 3, -3,  3>, < 3,  3,  3>, 0.2 }
  cylinder { <-3, -3,  3>, <-3,  3,  3>, 0.2 }
  cylinder { < 3, -3, -3>, < 3,  3, -3>, 0.2 }
  cylinder { <-3, -3, -3>, <-3,  3, -3>, 0.2 }

  cylinder { < 3,  3, -3>, < 3,  3,  3>, 0.2 }
  cylinder { <-3,  3, -3>, <-3,  3,  3>, 0.2 }
  cylinder { < 3, -3, -3>, < 3, -3,  3>, 0.2 }
  cylinder { <-3, -3, -3>, <-3, -3,  3>, 0.2 }
  
  pigment { color <1 0.7 0> }
  finish { diffuse 0.5 specular 1.0 }
}
*/

sky_sphere {
  pigment {
    granite
    color_map {
            [ 0.000  0.270 color rgb < 0, 0, 0> color rgb < 0, 0, 0> ]
            [ 0.270  0.290 color rgb <.5,.5,.4> color rgb <.8,.8,.4> ]
            [ 0.290  0.470 color rgb < 0, 0, 0> color rgb < 0, 0, 0> ]
            [ 0.470  0.490 color rgb <.4,.4,.5> color rgb <.4,.4,.8> ]
            [ 0.490  0.680 color rgb < 0, 0, 0> color rgb < 0, 0, 0> ]
            [ 0.680  0.700 color rgb <.5,.4,.4> color rgb <.8,.4,.4> ]
            [ 0.700  0.880 color rgb < 0, 0, 0> color rgb < 0, 0, 0> ]
            [ 0.880  0.900 color rgb <.5,.5,.5> color rgb < 1, 1, 1> ]
            [ 0.900  1.000 color rgb < 0, 0, 0> color rgb < 0, 0, 0> ]
    }
  turbulence 1
  sine_wave
  scale .2
  }
}

light_source {
  <-20, 50, 50> color White
}

sphere {
  <40, 60, 90>, 30
  texture {
    pigment {
      granite
      color_map {
        [0.0 0.3 color rgb <0.7, 0.3, 0.1> color rgb <0.6, 0.25, 0.1> ]
        [0.3 0.4 color rgb <0.6, 0.25, 0.1> color rgb <0.8, 0.5, 0.3> ]
        [0.4 1.0 color rgb <0.8, 0.5, 0.3> color rgb <0.6, 0.35, 0.3> ]
      }
      turbulence 1
      scale 60
    }
    finish {
      brilliance 0.1
      ambient 0.2
    }
  }
  texture {
    pigment {
      bozo
      color_map {
        [0.0 color rgbt <1, 1, 1, 1> ]
        [0.6 color rgbt <1, 1, 1, 1> ]
        [0.8 color rgbt <0.9, 0.9, 0.7, 0.3> ]
        [1.0 color rgbt <0.8, 0.8, 0.6, 0> ]
      }
      scale <5, 2, 5>
      rotate -30*z
    }
    finish {
      brilliance 0.1
      ambient 0.2
    }
  }
}

height_field {
  pgm
  "hf.pgm"
  hierarchy on
  scale <2,2,2>
  translate <-1,-0.6,-1>
  scale <-150,50,-150>
  pigment {
    agate
    scale 20
    color_map {
      [0.0 color rgb <1.0, 0.0, 0.0>]
      [1.0 color rgb <1.0, 0.8, 0.0>]
    }
  }
  normal {
    granite
    scale 3
  }
}

plane
{
  y, -12
  
  pigment {
    color Grey
  }
  
  finish {
    reflection 0.5
  }
}

fog {
  fog_type 2
  distance 40
  fog_offset -2.5
  fog_alt -5.0
  color rgb <0.2, 0.0, 0.4>
  up <0,-1,0>
}

union {
  torus {
    50, 0.5
  }
  torus {
    52, 0.5
  }
  torus {
    54, 0.5
  }
  torus {
    56, 0.5
  }
  torus {
    58, 0.5
  }
  rotate x*20
  rotate z*10
  translate <40, 60, 90>
  no_shadow
  pigment {
    color Yellow
  }
  finish {
    ambient 0.5
    diffuse 0.5
  }
}

sphere {
  <-43, -37, -40>, 20
  pigment {
    color Yellow
  }
}
