#!/bin/sh

# uncomment for debug
#set -x 

# check if we're on amd64 arch
if [ X`uname -m` != X"amd64" ]; then
	echo wrong arch, must be amd64 1>&2
	exit 1
fi

# check if we're root, we have to be to use vnconfig, disklabel and newfs
if [ `id -u` -ne 0 ]; then
	echo must be root 1>&2
	exit 1
fi

# $1 is disk image

if [ $# -lt 1 ]; then
	echo usage: create-fs.sh imagename 1>&2
	exit 1
fi

# if the raw image file doesn't exist create it otherwise set new=0
if [ ! -f $1 ]; then
	# create it
	vmctl create -s 100M $1
	new=1
else
	# it exists already
	new=0
fi

# check if we have a /mnt2 in the filesystem, use it.
if [ ! -d /mnt2 ];  then
	echo /mnt2 does not exist 1>&2
	exit 1
fi

# vnconfig the image
VNDEV=`vnconfig $1`

if [ $new -eq 1 ]; then
	# fdisk that image
	echo "fdisk'ing image $VNDEV"
	yes | fdisk -i $VNDEV
	if [ $? -ne 0 ]; then
		echo fdisk failed 1>&2
		echo unconfiguring $VNDEV 1>&2
		vnconfig -u $VNDEV
		exit 1
	fi
	# install a stupidly small disklabel
	disklabel -w -A $VNDEV 
	if [ $? -ne 0 ]; then
		echo disklabel failed 1>&2
		echo unconfiguring $VNDEV 1>&2
		vnconfig -u $VNDEV
		exit 1
	fi

	# newfs the a partition
	newfs /dev/r${VNDEV}a
	if [ $? -ne 0 ]; then
		echo newfs failed 1>&2
		echo unconfiguring $VNDEV 1>&2
		vnconfig -u $VNDEV
		exit 1
	fi
else
	echo not manipulating image $VNDEV, gettign disklabel...
	disklabel $VNDEV	
	
	if [ $? -ne 0 ]; then
		echo disklabel reports an error, bailing out.. 1>&2
		echo unconfiguring $VNDEV 1>&2
		vnconfig -u $VNDEV
		exit 1
	fi
fi

# mount the a partition under /mnt2 
echo mounting /dev/${VNDEV}a on /mnt2
mount /dev/${VNDEV}a /mnt2
if [ $? -ne 0 ]; then
	echo mount failed 1>&2
	echo unconfiguring $VNDEV 1>&2
	vnconfig -u $VNDEV
	exit 1
fi


# if we're new at this create a few directories with -p option to mkdir
if [ $new -eq 1 ]; then
	umask 022
	mkdir -p /mnt2/usr/local/share /mnt2/usr/share/misc /mnt2/usr/bin 
	mkdir -p /mnt2/usr/sbin
	mkdir -p /mnt2/usr/lib /mnt2/usr/libdata /mnt2/usr/libexec 
	mkdir -p /mnt2/dev /mnt2/etc /mnt2/home /mnt2/mnt /mnt2/bin 
	mkdir -p /mnt2/sbin /mnt2/var /mnt2/tmp
	chmod 1777 /mnt2/tmp
	
	# make devices here
	umask 022
	cp /dev/MAKEDEV /mnt2/dev/
	cd /mnt2/dev	
	./MAKEDEV all
 	if [ $? -ne 0 ]; then
		echo some devices failed to be made 1>&2
		exit 1
	fi
fi

# congrats finished, not unmounting /mnt2 or vnconfig -u'ing it.
echo at this point use the filesystem at your desire....the VND device is $VNDEV 
echo use vnconfig -u $VNDEV to unconfigure it after umounting /mnt2, exit...

exit 0
