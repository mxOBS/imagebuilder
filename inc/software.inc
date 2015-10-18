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
	distro=$2

	# collect packages to install
	unset PKGS

	# required packages for startup
	PKGS=("${PKGS[@]}" "kernel-3.14.y-fslc-imx6-sr" "u-boot-cubox-i" "initramfs-tools")

	# i.MX6 specific base system
	PKGS=("${PKGS[@]}" "irqbalance-imx" "bsp-cuboxi" "imx6-config" "runonce")

	# install useful software for a commandline environment
	PKGS=("${PKGS[@]}" "openssh-server" "e2fsprogs" "sudo" "fbset" "locales" "keyboard-configuration" "bash-completion" "command-not-found" "usbutils" "pciutils" "i2c-tools" "cpufrequtils" "rng-tools")
	[ "$distro" != "jessie" ] && PKGS=("${PKGS[@]}" "ntp")

	# prevent apt from running init-scripts
	block_services_enable $buildroot

	# install packages
	chroot_run $buildroot "apt-get -y install ${PKGS[@]}"

	# install galcore dummy packages, if they are available
	for pkg in $buildroot/var/cache/apt/archives/galcore-dummy*.deb; do
		test -e $pkg || continue
		name=`basename $pkg`
		chroot_run $buildroot "dpkg -i /var/cache/apt/archives/$name"
	done

	# update command-not-found cache
	chroot_run $buildroot /usr/sbin/update-command-not-found

	# install additional board support (bluetooth, network daemon, gpu, vpu, utilities
	PKGS=("alsa-utils" "bluez" "mediainfo" "connman" "connman-curses-git" "rfkill")
	[ "$distro" = "wheezy" ] && PKGS=("${PKGS[@]}" "imx-gpu-viv-fb" "imx-gpu-viv")
	[ "$distro" = "jessie" ] && PKGS=("${PKGS[@]}" "imx-gpu-viv-fb" "imx-gpu-viv" "gstreamer1.0-tools" "gstreamer1.0-imx-fb" "gstreamer1.0-plugins-good" "gstreamer1.0-plugins-bad" "gstreamer1.0-plugins-ugly")
	[ "$distro" = "trusty" ] && PKGS=("${PKGS[@]}" "imx-gpu-viv-fb" "imx-gpu-viv" "gstreamer1.0-tools" "gstreamer1.0-imx-fb" "gstreamer1.0-plugins-good")
	chroot_run $buildroot "apt-get -y install ${PKGS[@]}"

	# unblock
	block_services_disable $buildroot

	# add root filesystem to fstab
	# TODO: move to the part that actually creates the sdcard
	echo "/dev/root / ext4 defaults 0 0" >> $buildroot/etc/fstab

	# create uEnv.txt
	echo 'mmcroot=/dev/mmcblk0p1 rootwait rw' > $buildroot/boot/uEnv.txt
	echo 'mmcargs=setenv bootargs console=ttymxc0,115200n8 console=tty root=${mmcroot} quiet' >> $buildroot/boot/uEnv.txt

	# resize FS on first boot
	cat > $buildroot/etc/runonce.d/expand-fs.sh << EOF
#!/bin/sh
/bin/bash /usr/share/imx6-config/expand-fs /dev/mmcblk0 /dev/mmcblk0p1
s=\$?
if [ \$s = 2 ]; then
	runonce-helper add expand-fs-stage2 /bin/bash /usr/share/imx6-config/expand-fs /dev/mmcblk0 /dev/mmcblk0p1 2
	exit 0
fi
exit \$s
EOF

	# default to multi-user CLI target
	chroot_run $buildroot systemctl set-default -f multi-user.target

	# enable systemd time synchronization
	[ "$distro" = "jessie" ] && chroot_run $buildroot systemctl enable systemd-timesyncd.service
}

collect_x11() {
	distro=$1

	# collect packages
	unset PKGS

	# core Xorg
	PKGS=("${PKGS[@]}" "xserver-xorg-core" "xinit" "lightdm")

	# Hardware acceleration packages and codecs
	[ "$distro" = "wheezy" ] && PKGS=("${PKGS[@]}" "imx-gpu-viv" "imx-gpu-viv-x11" "xserver-xorg-video-imx-viv" "gstreamer0.10-imx" "gstreamer0.10-plugins-bad" "gstreamer0.10-plugins-base" "gstreamer0.10-plugins-good" "gstreamer0.10-plugins-ugly" "mesa-utils")
	[ "$distro" = "jessie" ] && PKGS=("${PKGS[@]}" "imx-gpu-viv" "imx-gpu-viv-x11" "xserver-xorg-video-imx-viv" "gstreamer1.0-imx-x11" "gstreamer1.0-plugins-base" "gstreamer1.0-plugins-good" "gstreamer1.0-plugins-bad" "gstreamer1.0-plugins-ugly" "mesa-utils" "mesa-utils-extra")
	[ "$distro" = "trusty" ] && PKGS=("${PKGS[@]}" "imx-gpu-viv" "imx-gpu-viv-x11" "xserver-xorg-video-imx-viv" "gstreamer1.0-imx-x11")

	echo ${PKGS[@]}
}

collect_mate() {
	distro=$1

	PKGS=(`collect_x11 $distro`)

	PKGS=("${PKGS[@]}" "mate-desktop-environment" "blueman" "connman-ui")

	echo ${PKGS[@]}
}

collect_xfce() {
	distro=$1

	PKGS=(`collect_x11 $distro`)

	PKGS=("${PKGS[@]}" "xfce4" "xfce4-terminal" "connman-ui")

	# install a few default apps
	PKGS=("${PKGS[@]}" "snappy" "abiword" "gnumeric" "xpdf" "xchat")

	echo ${PKGS[@]}
}

# todo: kde, gnome3

install_desktop() {
	buildroot=$1
	distro=$2
	type=$3

	[ "$type" = "cli" ] && return 0

	# collect packages
	unset PKGS
	[ "$type" = "mate" ] && PKGS=("${PKGS[@]}" "`collect_mate $distro`")
	[ "$type" = "xfce" ] && PKGS=("${PKGS[@]}" "`collect_xfce $distro`")

	# some default applications
	[ "$distro" = "wheezy" ] || [ "$distro" = "jessie" ] && PKGS=("${PKGS[@]}" "iceweasel" "icedove")
	[ "$distro" = "wheezy" ] && PKGS=("${PKGS[@]}" "chromium-browser")
	[ "$distro" = "trusty" ] && PKGS=("${PKGS[@]}" "chromium-browser")

	# install packages
	block_services_enable $buildroot
	# HACK
	chroot_run $buildroot "apt-get -y remove gstreamer1.0-imx-fb"
	chroot_run $buildroot "apt-get -y install ${PKGS[@]}"
	block_services_disable $buildroot

	# set boot target to graphical desktop
	chroot_run $buildroot systemctl set-default graphical.target
}
