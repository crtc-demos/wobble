#!/bin/sh
vqencode() {
  local file="$1"
  local alpha="$2"
  if [ "$alpha" ]; then
    echo Converting $file with alpha channel
  else
    echo Converting $file without alpha
  fi
  vqenc --twiddle --highq --kmg $alpha "$file"
}

vqencode sky1.png
vqencode sky2o.png --alpha
vqencode sky3.png
vqencode sky4.png
vqencode sky5.png
vqencode sky6.png
vqencode sky7.png
vqencode sky8.png
