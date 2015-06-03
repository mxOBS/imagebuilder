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
	PKGS=("${PKGS[@]}" "openssh-server" "sudo" "ca-certificates" "ntp" "fbset" "alsa-utils" "bluez")

	# prevent apt from running init-scripts
	block_services_enable $buildroot

	# install packages
	chroot_run $buildroot "apt-get -y install ${PKGS[@]}"

	# install galcore dummy packages, if they are available
	for pkg in $buildroot/var/cache/apt/archives/galcore-dummy*.deb; do
		name=`basename $pkg`
		chroot_run $buildroot "dpkg -i /var/cache/apt/archives/$name"
	done

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

collect_x11() {
	distro=$1

	# collect packages
	PKGS=

	# core Xorg
	PKGS=("${PKGS[@]}" "xserver-xorg-core" "xinit" "lightdm")

	# Hardware acceleration packages
	[ "$distro" = "wheezy" ] && PKGS=("${PKGS[@]}" "gpu-viv-bin-v4-x11" "xserver-xorg-video-imx-viv" "gstreamer0.10-imx")
	[ "$distro" = "jessie" ] && PKGS=("${PKGS[@]}" "imx-gpu-viv" "imx-gpu-viv-x11" "xserver-xorg-video-imx-viv" "gstreamer1.0-imx-x11")
	[ "$distro" = "trusty" ] && PKGS=("${PKGS[@]}" "imx-gpu-viv" "imx-gpu-viv-x11" "xserver-xorg-video-imx-viv" "gstreamer1.0-imx-x11")

	echo ${PKGS[@]}
}

collect_mate() {
	distro=$1

	PKGS=(`collect_x11 $distro`)

	PKGS=("${PKGS[@]}" "mate-desktop-environment" "blueman")

	echo ${PKGS[@]}
}

collect_xfce() {
	distro=$1

	PKGS=(`collect_x11 $distro`)

	PKGS=("${PKGS[@]}" "xfce4")

	echo ${PKGS[@]}
}

# todo: kde, gnome3

install_desktop() {
	buildroot=$1
	distro=$2
	type=$3

	[ "$type" = "cli" ] && return 0

	# collect packages
	PKGS=
	[ "$type" = "mate" ] && PKGS=("${PKGS[@]}" "`collect_mate $distro`")
	[ "$type" = "xfce" ] && PKGS=("${PKGS[@]}" "`collect_xfce $distro`")

	# some default applications
	PKGS=("${PKGS[@]}" "iceweasel" "icedove")
	test $distro -eq wheezy -o $distro = trusty && PKGS=("${PKGS[@]}" "chromium-browser")

	# install packages
	block_services_enable $buildroot
	chroot_run $buildroot "apt-get -y install ${PKGS[@]}"
	block_services_disable $buildroot
}
