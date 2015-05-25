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

block_services_enable() {
	buildroot=$1

	echo "exit 101" > $buildroot/usr/sbin/policy-rc.d
	chmod 777 $buildroot/usr/sbin/policy-rc.d
}

block_services_disable() {
	buildroot=$1

	rm -f $buildroot/usr/sbin/policy-rc.d
}

# usage: <buildroot>
install_base() {
	buildroot=$1

	# collect packages to install
	PKGS=

	# required packages for startup
	PKGS=("${PKGS[@]}" "kernel-cubox-i" "u-boot-cubox-i" "initramfs-tools")

	# i.MX6 specific base system
	PKGS=("${PKGS[@]}" "irqbalance-imx" "bsp-cuboxi" "imx6-config")

	# install useful software for a commandline environment
	PKGS=("${PKGS[@]}" "openssh-server" "sudo" "ca-certificates" "ntp" "fbset" "alsa-utils")

	# prevent apt from running init-scripts
	block_services_enable $buildroot

	# install packages
	chroot_run $buildroot "apt-get -y install ${PKGS[@]}"

	# unblock
	block_services_disable $buildroot

	# create initramfs
	for kernel in $buildroot/boot/zImage-*; do
		localversion=`basename $kernel | sed -e "s;^zImage-;;g"`
		chroot_run $buildroot "/bin/bash /usr/sbin/update-initramfs -c -k $localversion"
	done

	# create boot symlinks
	for kernel in $buildroot/boot/zImage-*; do
		localversion=`basename $kernel | sed -e "s;^zImage-;;g"`
		ln -sv zImage-$localversion $buildroot/boot/zImage
		ln -sv initrd.img-$localversion $buildroot/boot/initrd
		break
	done

	# add root filesystem to fstab
	# TODO: move to the part that actually creates the sdcard
	echo "/dev/mmcblk0p1 / ext4 defaults 0 0" >> build/etc/fstab

	# create uEnv.txt
	echo "mmcroot=/dev/mmcblk0p1 rootwait rw enable_wait_mode=off" > $buildroot/boot/uEnv.txt
}

install_x11() {
	buildroot=$1

	# install galcore dummy packages, if they are available
	for pkg in $buildroot/var/cache/apt/archives/galcore-dummy*.deb; do
		name=`basename $pkg`
		chroot_run $buildroot "dpkg -i /var/cache/apt/archives/$name"
	done

	block_services_enable $buildroot
	chroot_run $buildroot "apt-get -y install imx-gpu-viv imx-gpu-viv-x11 xserver-xorg-video-imx-viv xserver-xorg-core gstreamer1.0-imx xinit lightdm"
	block_services_disable $buildroot
}

install_mate() {
	buildroot=$1

	install_x11 $buildroot

	block_services_enable $buildroot
	chroot_run $buildroot "apt-get -y install mate-desktop-environment icedove"
	block_services_disable $buildroot
}

# todo: xfce, kde, gnome3

install_desktop() {
	buildroot=$1
	type=$2

	if [ "$type" = "mate" ]; then
		install_mate $buildroot
	fi
}
