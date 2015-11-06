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

function qemu_setup() {
	buildroot=$1

	if [[ `uname -m` != arm* ]]; then
		# some systems, e.g. debian, have just 1 binary for qemu-user:
		if [ -e /usr/bin/qemu-arm-static ]; then
			cp /usr/bin/qemu-arm-static $buildroot/usr/bin/
			return
		fi

		# others, e.g. opensuse,  use 2:
		if [ -e /usr/bin/qemu-arm-binfmt ] && [ -e /usr/bin/qemu-arm ]; then
			cp /usr/bin/qemu-arm-binfmt $buildroot/usr/bin/
			cp /usr/bin/qemu-arm $buildroot/usr/bin/
			return
		fi

		echo "WARNING: Failed to find qemu binary!"
	fi
}

function qemu_remove() {
	buildroot=$1

	if [[ `uname -m` != arm* ]]; then
		if [ -e /usr/bin/qemu-arm-static ]; then
			rm $buildroot/usr/bin/qemu-arm-static
			return
		fi
		if [ -e /usr/bin/qemu-arm-binfmt ] && [ -e /usr/bin/qemu-arm ]; then
			rm $buildroot/usr/bin/qemu-arm-binfmt
			rm $buildroot/usr/bin/qemu-arm
			return
		fi
	fi
}

# HACK
# define qemu-linux-user as a dependency if its not found
dependencies=cp
[[ `uname -m` != arm* ]] && [ ! -e /usr/bin/qemu-arm-static ] && [ ! -e /usr/bin/qemu-arm-binfmt ] && [ ! -e /usr/bin/qemu-arm ] && dependencies="$dependencies qemu-linux-user"
