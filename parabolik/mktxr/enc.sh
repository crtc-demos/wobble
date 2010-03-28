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

#vqencode sky1.png
#vqencode sky2o.png --alpha
vqencode sky23.png
vqencode sky24.png
vqencode sky25.png
vqencode sky26.png
vqencode sky27.png
vqencode sky28.png
