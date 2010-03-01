#!/bin/sh
SIZE=512

SIZEM1=$(( $SIZE - 1 ))
CENT=$(( $SIZE / 2 ))
povray res640.ini +I sky2.pov
convert sky2.png -type TrueColorMatte -depth 8 \( -clone 0 -channel Red -separate \) \( -clone 0 -channel Green -separate \) \( -clone 0 -channel Blue -separate \) \( -clone 0 -fill "#fff" -draw "rectangle 0,0,$SIZEM1,$SIZEM1" -fill "#000" -draw "circle $CENT,$CENT,2,$CENT" \) -delete 0 -channel RGBA -combine sky2o.png
