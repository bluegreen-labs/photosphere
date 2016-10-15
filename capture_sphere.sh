#!/bin/bash

# Snaps THETA S pictures
# uploads them and deletes the
# images on the camera to free up memory
# needs to be called using crontab for a time lapse
# implementation

# if not arguments
if [ "$*" == "" ]; then
    echo "No arguments provided"
    echo "use: -u true / false (upload)"
    echo "use: -n true / false (night mode)"
   	echo "use: -s xxx.xxx.xxx.xxx (ip address / domain name)"
    exit 1
fi

# query arguments
while getopts ":h?u:n:s:" opt; do
    case "$opt" in
    h|\?)
   		echo "No arguments provided"
    	echo "use: -u true / false (upload)"
    	echo "use: -n true / false (night mode)"
    	echo "use: -s xxx.xxx.xxx.xxx (ip address / domain name)"
    	exit 0
        ;;
    u)  
    	upload=$OPTARG
    	;;
    n)  
    	nightmode=$OPTARG
    	;;
    	
    s)  
    	server=$OPTARG
    	;;
    :)
    	echo "Option -$OPTARG requires an argument." >&2
    	exit 1
    	;;
    esac
done

# set paths explicitly
PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games"

# set working directory
# in which to save the data
#cd ~/tmp/ # for testing only
cd /var/tmp/

# make sure to unmount the camera
# basically in text mode it should not
# auto mount (as far as I know)

# cameraname
camera="virtualforest"
datetime=` date +%Y_%m_%d_%H%M%S`

# check if the camera is connected
# if not exit cleanly
status=`ptpcam -i | grep "serial" | wc -l`

if [ "$status" = "0" ];
then
	echo ""
	echo "Your camera is not active, connected or mounted as a drive!"
	echo "-----------------------------------------------------------"
	echo "Check your usb cable, turn the camera on and make sure"
	echo "that your camera is not mounted as a drive."
	echo ""
	exit 0
fi

# check the battery status
battery=`ptpcam --show-property=0x5001 | grep "to:" | awk '{print $6}'`

# output battery status to file
echo $battery >> battery_status.txt

if [ "$battery" -lt 20 ];
then
	echo "low battery"
	exit 0
fi

# set the timeout to indefinite
# as well as the sleep delay
# this prevents timed shutdowns
ptpcam --set-property=0xd803 --val=0 # sleep delay
ptpcam --set-property=0xd802 --val=0 # set auto power off
ptpcam --set-property=0x502c --val=100 # set shutter sound to max

# list all files on the device
handle=`ptpcam -L | grep 0x | awk '{print $1}' | sed 's/://g'`


# clear all files on the device before capturing
# any new data
do
	ptpcam --delete-object=$i
done

if [[ "$nightmode" == "FALSE" || "$nightmode" == "F" ]] || [[ "$nightmode" == "f" || "$nightmode" == "false" ]];
	then

	# loop over 3 exposure settings
	exposures="2000 0 -2000"
	
	for exp in $exposures;do
		# Change settings
		ptpcam --set-property=0x500E --val=0x8003 # ISO priority
		ptpcam --set-property=0x5005 --val=0x8001 # set WB
		ptpcam --set-property=0x500F --val=100 # set ISO
		ptpcam --set-property=0x5010 --val=$exp # set compensation
		
		# snap picture
		# and wait for it to complete (max 60s)
		ptpcam -c
		sleep 60
	
		# list last file
		handle=`ptpcam -L | grep 0x | awk '{print $1}' | sed 's/://g'`
		echo $handle
		filename=`ptpcam -L | grep 0x | awk '{print $5}'`

		# grab the last file
		ptpcam --get-file=$handle

		# grab filename of the last downloaded file
		filename=`ls -t | head -1`

		# create filename based upon time / date
		newfilename=`echo $camera\_exp$exp\_$datetime.jpg`

		# rename the last file created
		mv $filename $newfilename

		# remove file
		ptpcam --delete-object=$handle
	done

else 
	# nightmode
	# not working yet, doesn't accept the
	# shutter speeds !!!!!
	
	# Change settings
	ptpcam --set-property=0x500E --val=0x0004 # ISO priority
	ptpcam --set-property=0x5005 --val=0x8001 # set WB
	ptpcam --set-property=0xD00F --val=30,1 # set shutter
	
	# snap picture
	# and wait for it to complete (max 60s)
	ptpcam -c
	sleep 60
	
	# list last file

	handle=`ptpcam -L | grep 0x | awk '{print $1}' | sed 's/://g'`
	filename=`ptpcam -L | grep 0x | awk '{print $5}'`

	# grab the last file
	ptpcam --get-file=$handle

	# create filename based upon time / date
	newfilename=`echo $camera\_exp0\_$datetime.jpg`

	# rename the last file created
	mv $filename $newfilename

	# remove file
	ptpcam --delete-object=$handle
fi

# upload new data to an image server
if [[ "$upload" == "TRUE" || "$upload" == "T" ]] \
|| [[ "$upload" == "t" || "$upload" == true ]]
then
	lftp ftp://anonymous:anonymous@${server} -e "mirror --verbose --reverse --Remove-source-files ./ ./data/ ;quit"
fi

