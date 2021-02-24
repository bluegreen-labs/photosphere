#!/bin/bash

# Snaps THETA Z1 pictures
# uploads them and deletes the
# images on the camera to free up memory
# needs to be called using crontab for a time lapse
# implementation

# if not arguments
if [ "$*" == "" ]; then
    echo "No arguments provided"
    echo "use: -u true / false (upload)"
    echo "use: -n true / false (night mode)"
    exit 1
fi

# query arguments
while getopts ":h?u:n:" opt; do
    case "$opt" in
    h|\?)
   		echo "No arguments provided:"
    	echo "use: -u true / false (upload)"
    	echo "use: -n true / false (nightmode)"
    	exit 0
        ;;
    u)  
    	upload=$OPTARG
    	;;
    n)  
    	nightmode=$OPTARG
    	;;
    :)
    	echo "Option -$OPTARG requires an argument." >&2
    	exit 1
    	;;
    esac
done

# set paths explicitly
PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games"
TMPDIR="/var/tmp"

# set working directory
# in which to save the data
cd $TMPDIR

# download github repo if not present
#  update panorama (overwrite whole repo)
# removing all previous commits
if [ ! -d "./theta-z1" ];
 then
  echo "Cloning image repo..."
  # clone repo (change to yours and install ssh access)
  git clone git@github.com:bluegreen-camera/theta-z1.git
fi

# make sure to unmount the camera
# basically in text mode it should not
# auto mount (as far as I know)

# cameraname
camera="virtualforest"
datetime=` date +%Y_%m_%d_%H%M%S`

# check if the camera is connected
# if not exit cleanly
status=`ptpcam --show-property=0x5001 | grep "ERROR" | wc -l`

if [ "$status" = "1" ];
then
	echo ""
	echo "Your camera is not active"
	echo "-----------------------------------------------------------"
	echo "activating your camera"
	echo ""
	echo ""
	
	# activate the camera
	boot=`ptpcam --set-property=0xD80E --val=0x00 | grep "succeeded" | wc -l`
	
	if [ "$boot" == 1 ];
	then
         echo "booting the device... (waiting 10s)"
	 sleep 10
	else
         exit 0
	fi
fi

# check the battery status
battery=`ptpcam --show-property=0x5001 | grep "to:" | awk '{print $6}'`

# output battery status to file
echo $battery > battery_status.txt

if [ "$battery" -lt 10 ];
then
	echo "low battery"
	ptpcam --set-property=0xD80E --val=0x01 # go to sleep
	sleep 1
	exit 0
fi

# set the timeout to indefinite
# as well as the sleep delay
# this prevents timed shutdowns
#ptpcam --set-property=0xd803 --val=0 # sleep delay
#ptpcam --set-property=0xd802 --val=0 # set auto power off
ptpcam --set-property=0x502c --val=0 # set shutter sound to min 0 or max 100

# list all files on the device
handle=`ptpcam -L | grep 0x | awk '{print $1}' | sed 's/://g'`

# clear all files on the device before capturing
# any new data
for i in $handle;do
	ptpcam --delete-object=$i
done


if [[ "$nightmode" == "FALSE" || "$nightmode" == "F" ]] ||
 [[ "$nightmode" == "f" || "$nightmode" == "false" ]];
 then
         
        # loop over 2 exposure settings
        exposures="0 -2000"
	
        ptpcam --set-property=0x500E --val=0x8003 # ISO priority
        ptpcam --set-property=0x5005 --val=0x8002 # set WB to cloudy
        ptpcam --set-property=0x500F --val=100 # set ISO
	
        for exp in $exposures;do
	        # Change settings of exposure compensation
	        ptpcam --set-property=0x5010 --val=$exp # set compensation
		
	        # snap picture
	        # and wait for it to complete (max 60s)
	        ptpcam -c
	        sleep 60
	
	        # list last file
	        handle=`ptpcam -L | grep 0x | awk '{print $1}' | sed 's/://g'`
	
	        # grab the last file
	        # wait a bit otherwise the next
	        # commands are too fast
	        ptpcam --get-file=$handle
	        sleep 10

	        # grab filename of the last downloaded file
	        filename=`ls -t *.JPG | head -1`

	        # create filename based upon time / date
	        newfilename=`echo $camera\-exp$exp\_$datetime.jpg`

	        # rename the last file created
	        mv $filename $newfilename

	        # remove file
	        ptpcam --delete-object=$handle
        done

else
                
        ptpcam --set-property=0x500E --val=0x0002 # set normal
        ptpcam --set-property=0x5005 --val=0x8002 # set WB to cloudy
        
        # snap picture
        # and wait for it to complete (max 60s)
        ptpcam -c
        sleep 60
	
        # list last file
        handle=`ptpcam -L | grep 0x | awk '{print $1}' | sed 's/://g'`
	
        # grab the last file
        # wait a bit otherwise the next
        # commands are too fast
        ptpcam --get-file=$handle
        sleep 10

        # grab filename of the last downloaded file
        filename=`ls -t *.JPG | head -1`

        # create filename based upon time / date
        newfilename=`echo $camera\-exp0\_$datetime.jpg`

        # rename the last file created
        mv $filename $newfilename

        # remove file
        ptpcam --delete-object=$handle
        sleep 2
fi

# put camera into sleep mode
ptpcam --set-property=0xD80E --val=0x01 # go to sleep

# upload new data to an image server
if [[ "$upload" == "TRUE" || "$upload" == "T" ]] \
|| [[ "$upload" == "t" || "$upload" == true ]]
then
        
        # create panorama to visualized
        # with masked out section
        if [ ! -f ~/photosphere/mask.png ];
        then
	        echo "no mask file -mask.png- in current directory"
	        rm *.jpg
	        exit 1
	else		
          convert $camera\-exp0\_$datetime.jpg ~/photosphere/mask.png\
           -compose Multiply -composite ./theta-z1/panorama.jpg
        fi
        
        # move data and move into
        # git directory
        cd theta-z1
        
        # checkout latest
        git checkout --orphan latest_branch
        git add -A
        git commit -am "update"
        
        # delete main branch, rename current one
        # to main
        git branch -D main
        git branch -m main
        
        # push new "main" to origin
        git push -f origin main
        
        # go to base directory
        cd $TMPDIR
fi

# remove all files
rm *.jpg

# done
exit 0
