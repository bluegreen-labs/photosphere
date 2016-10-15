#!/bin/bash
# generates a "smooth" interpolated movie
# of a sequence of images (not limited to spherical ones)

# 30 fps, so this means each original image corresponds to one second of video.
frames_per_image=30
fps=30

# takes two arguments (input and output directory)
# no verifications are done to see if either exists!
dir=$1
out=$2

# list all images (ending in .jpg)
images=`ls *.jpg`

# Iterate through the images keeping track of the previous one to be blended
#  with the current one.
num=0   # frame number
prev='' # previous image or empty string if none
for image in $images;
do	
	# If there is a previous image, blend with it.
	if [ -f "$prev" ]
	then
		for fade_amount in $(seq 1 $((frames_per_image - 1)))
		do
			# Use bc for math to get decimals.
			composite -blend "$(echo "$fade_amount/$frames_per_image*100" | bc -lq)" "$image" "$prev" "$dir/image$num"".png"
			num=$((num + 1))
		done
	fi

	# Always include the actual image as one of the frames.
	# Use convert so any image format will work.
	convert "$image" "$dir/image${num}.png"
	num=$((num + 1))

	# Remember this image for the next iteration.
	prev="$image"
done

# Encode the video from the frames.
# -pix_fmt yuv420p is normal pixel format, but default is yuv444p which
#  ffmpeg warns might not be supported by all players.
ffmpeg -i "$dir/image%d.png" -c:v libx264 -pix_fmt yuv420p -r "$fps" "$out"

# quit
exit 0