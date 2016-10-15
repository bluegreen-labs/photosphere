#!/bin/bash

# mask the sphere bottom before processing into a movie
# loop over all jpgs in the current directory
# it assumes the mask file is in place if not 
# the script bails

if [ ! -f mask.png ];
then
	echo "no mask file -mask.png- in current directory" 
	exit 1
fi

for f in *.jpg
do   
	convert $f -modulate 100,130,100 -sharpen 0x2 mask.png -compose Multiply -composite $f
done

exit 0
