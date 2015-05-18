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

	debootstrap --no-check-gpg --arch=armhf $distro build $mirror
}
