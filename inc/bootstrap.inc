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

	# create, or reuse tarball of packages
	if [ -e $distro.bootstrap.tar ]; then
		TBARGS="--unpack-tarball=$distro.bootstrap.tar"
	else
		TBAGRS="--make-tarball=$distro.bootstrap.tar"
	fi

	# handle the need for qemu
	if [[ `uname -m` = arm* ]]; then
		debootstrap --no-check-gpg --arch=armhf $TBARGS $distro build $mirror
	else
		# first stage
		debootstrap --no-check-gpg --foreign --arch=armhf $TBARGS $distro build $mirror

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
