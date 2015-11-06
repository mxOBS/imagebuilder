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

# check arguments
usage() {
	echo "Usage: $0 <configuration>"
	echo "configuration: [configs/*]"
}
if [ $# != 1 ]; then
	usage
	exit 1
fi

# see if config file exists
configfile="$1"
if [ ! -e "$configfile" ]; then
	echo "Error: $configfile does not exist!"
	exit 1
fi

# check build environment

if [ "x$UID" != "x0" ]; then
	echo "Error: This script must run as root!"
	exit 1
fi

# load all modules
for m in inc/*.inc; do
	dependencies=""
	# load each module
	source $m

	# and check its dependencies
	[ -z "$dependencies" ] && for dep in $dependencies; do
		which $program 1>/dev/null 2>/dev/null
		if [ $? != 0 ]; then
			echo "Error: $program required but not found!"
			exit 1
		fi
	done
done

# choose build directory name
buildroot="build-$(basename "$configfile" .inc)"
if [ -e "$buildroot" ]; then
	echo "Warning: build-directory exists already and will be deleted!"
	echo "This is your chance to cancel. Press enter to procede."
	read
	rm -rf $buildroot
fi

# perform build
source "$configfile"

# make tarball
tarballname="$(basename "$configfile" .inc)"
rm -f "$tarballname.tar"
pushd "$buildroot"; tar -cf "../$tarballname.tar" *; popd

echo "Finished creating $tarballname.tar!"
