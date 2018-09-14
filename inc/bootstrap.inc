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

# usage: <buildroot> <distro-codename> <packages>
bootstrap() {
	buildroot=$1
	distro=$2
	arch=$3
	shift 3
	packages="$(echo $* | sed -e "s; ;,;g")"
	INCLUDE_PACKAGES_ARG=$(test -n "$packages" && echo "--include=$packages")

	# select distribution-specific settings
	mirror=
	if [ "$distro" = "wheezy" ]; then
		mirror="http://httpredir.debian.org/debian"
	fi
	if [ "$distro" = "jessie" ]; then
		mirror="http://httpredir.debian.org/debian"
	fi
	if [ "$distro" = "trusty" ]; then
		mirror="http://ports.ubuntu.com/ubuntu-ports"
	fi

	# create tarball of packages if it doesnt exist yet
	if [ ! -e $distro.bootstrap.tar ]; then
		debootstrap --no-check-gpg --arch=$arch $INCLUDE_PACKAGES_ARG --make-tarball=$PWD/$distro.bootstrap.tar --variant=minbase $distro $buildroot $mirror
	fi

	# on foreign architectures, run second stage manually
	if [[ `uname -m` = arm* ]]; then
		debootstrap --no-check-gpg --arch=$arch --unpack-tarball=$PWD/$distro.bootstrap.tar $INCLUDE_PACKAGES_ARG $distro $buildroot $mirror
	else
		# first stage
		debootstrap --no-check-gpg --foreign --arch=$arch --unpack-tarball=$PWD/$distro.bootstrap.tar $INCLUDE_PACKAGES_ARG $distro $buildroot $mirror

		# second stage in chroot
		chroot_run $buildroot "/bin/bash /debootstrap/debootstrap --second-stage"

		# third stage
		# copy resolv.conf
		cp /etc/resolv.conf $buildroot/etc/
	fi

	# apply permission hacks
	chmod +s "$buildroot/bin/ping" "$buildroot/bin/ping6"

	# restore previous apt cache if any
	aptcache_restore "$buildroot"

	# set default locale
	sed -i "s;# en_US.UTF-8 UTF-8;en_US.UTF-8 UTF-8;g" $buildroot/etc/locale.gen
	chroot_run $buildroot /usr/sbin/locale-gen
	chroot_run $buildroot /usr/sbin/update-locale --no-checks LANG
	chroot_run $buildroot /usr/sbin/update-locale "LANG=en_US.UTF-8"
}

# define required programs
dependencies="debootstrap cp"
