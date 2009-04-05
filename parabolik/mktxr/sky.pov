// Persistence Of Vision raytracer version 3.0 sample file.
// Use copies of this file for starting your own scenes.

#version 3.5;

global_settings { assumed_gamma 2.2 }

#include "colors.inc"
#include "textures.inc"
#include "stars.inc"
#include "skies.inc"

camera
{
  orthographic
#switch (clock)
  #case (1)
	location  <0.0, 0.5, -1.0>
        look_at   <0.0, 0.5, 0.0>
	up        y
	right     x
  #break
  #case (2)
        location  <0.0, 0.5, 1.0>
        look_at   <0.0, 0.5, 0.0>
	up        -y
	right     -x
  #break
#end
}



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
  <40, 30, 90>, 30
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
  scale <150,20,150>
  pigment {
    bozo
    color_map {
      [0.0 color rgb <0.7, 0.3, 0>]
      [1.0 color rgb <0.8, 0.5, 0>]
    }
  }
  normal {
    bumps
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
  translate <40, 30, 90>
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
