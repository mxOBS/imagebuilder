# usage: <buildroot> <distro-codename>
bootstrap_system() {
	buildroot=$1
	distro=$2

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
		debootstrap --no-check-gpg --arch=armhf --make-tarball=$PWD/$distro.bootstrap.tar $distro build $mirror
	fi

	# handle the need for qemu
	if [[ `uname -m` = arm* ]]; then
		debootstrap --no-check-gpg --arch=armhf --unpack-tarball=$PWD/$distro.bootstrap.tar $distro build $mirror
	else
		# first stage
		debootstrap --no-check-gpg --foreign --arch=armhf --unpack-tarball=$PWD/$distro.bootstrap.tar $distro build $mirror

		# copy qemu
		qemu_setup $buildroot

		# second stage in chroot
		chroot $buildroot /bin/bash /debootstrap/debootstrap --second-stage

		# remove qemu again
		qemu_remove $buildroot

		# third stage
		# copy resolv.conf
		cp /etc/resolv.conf $buildroot/etc/
	fi
}
