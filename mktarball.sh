#!/bin/bash

# load all modules
for m in inc/*.inc; do
	source $m
done

# check arguments
usage() {
	echo "Usage: $0 <distro-codename> <image-type>"
	echo "distro-codename: [wheezy,jessie,trusty]"
	echo "image-type: [cli,xfce,mate]"
}
if [ $# != 2 ]; then
	usage
	exit 1
fi

distro=$1
if [ "$distro" != "wheezy" ] && [ "$distro" != "jessie" ] && [ "$distro" != "trusty" ]; then
	usage
	exit 1
fi

type=$2
if [ "$type" != "cli" ] && [ "$type" != "mate" ] && [ "$type" != "xfce" ]; then
	usage
	exit 1
fi

# check build environment

if [ "x$UID" != "x0" ]; then
	echo "Error: This script must run as root!"
	exit 1
fi

which debootstrap 1>/dev/null 2>/dev/null
if [ $? != 0 ]; then
	echo "Error: debootstrap is not installed!"
	exit 1
fi

if [ -e build ]; then
	echo "Warning: build-directory exists already and will be deleted!"
	echo "This is your chance to cancel. Press enter to procede."
	read
	rm -rf build
fi

# bootstrap system
bootstrap_system build $distro

# add repos
add_repos build $distro

# restore apt cache from previous runs
restore_aptcache build

# install software selection
install_base build $distro

install_desktop build $distro $type

# save apt cache for later use
save_aptcache build

# configure system
configure_system build

# remove traces of build-system
cleanup_system build

# make tarball
rm -f $distro.tar
pushd build; tar -cf ../$distro.tar *; popd

echo "Finished creating $distro.tar!"
