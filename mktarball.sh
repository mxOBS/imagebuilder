#!/bin/bash
# 
# Copyright (c) 2015 Josua Mayer
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# 

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

buildroot=build-$distro-$type
if [ -e $buildroot ]; then
	echo "Warning: build-directory exists already and will be deleted!"
	echo "This is your chance to cancel. Press enter to procede."
	read
	rm -rf $buildroot
fi

# bootstrap system
bootstrap_system $buildroot $distro

# add repos
add_repos $buildroot $distro

# restore apt cache from previous runs
restore_aptcache $buildroot

# install software selection
install_base $buildroot $distro

# install downstream packages
install_local $buildroot

install_desktop $buildroot $distro $type

# save apt cache for later use
save_aptcache $buildroot

# configure system
configure_system $buildroot $distro

# remove traces of build-system
cleanup_system $buildroot

# make tarball
rm -f $distro.tar
pushd $buildroot; tar -cf ../$distro.tar *; popd

echo "Finished creating $distro.tar!"
