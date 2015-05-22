function restore_aptcache() {
	buildroot=$1

	if [ -e $distro.pkgcache.tar ]; then
		tar -C $buildroot -xf $distro.pkgcache.tar
	fi
}

function save_aptcache() {
	buildroot=$1

	curdir="$PWD"
	pushd $buildroot
	tar -cf "$curdir/$distro.pkgcache.tar" var/cache/apt/archives/*.deb
	popd
}

# usage: <buildroot>
install_base() {
	buildroot=$1

	# collect packages to install
	PKGS=

	# required packages for startup
	PKGS=("${PKGS[@]}" "kernel-cubox-i" "u-boot-cubox-i")

	# i.MX6 specific base system
	PKGS=("${PKGS[@]}" "irqbalance-imx" "bsp-cuboxi" "imx6-config")

	# install useful software for a commandline environment
	PKGS=("${PKGS[@]}" "openssh-server" "sudo" "ca-certificates" "ntp" "fbset")

	# HACK: prevent apt from running init-scripts
	echo "exit 101" > $buildroot/usr/sbin/policy-rc.d
	chmod 777 $buildroot/usr/sbin/policy-rc.d

	# install packages
	chroot_run $buildroot "apt-get -y install ${PKGS[@]}"

	# UNHACK
	rm -f $buildroot/usr/sbin/policy-rc.d

	# select kernel to boot
	# TODO: integrate with package
	pushd $buildroot/boot; ln -sv zImage-* zImage; popd

	# add root filesystem to fstab
	# TODO: move to the part that actually creates the sdcard
	echo "/dev/mmcblk0p1 / ext4 defaults 0 0" >> build/etc/fstab

	# create uEnv.txt
	echo "mmcroot=/dev/mmcblk0p1 rootwait rw" > $buildroot/boot/uEnv.txt
}

# TODO: desktops
