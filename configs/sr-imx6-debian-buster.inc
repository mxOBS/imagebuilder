# 
# Copyright (c) 2017-2019 Josua Mayer
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

# This is the actual cookbook

# bootstrap a debian system
bootstrap "$buildroot" buster armhf apt-transport-https ca-certificates locales systemd-sysv initramfs-tools busybox curl

# add debian repos
deleterepos "$buildroot" sources.list
addrepo "$buildroot" sources.list http://deb.debian.org/debian buster main non-free
addrepo "$buildroot" sources.list http://deb.debian.org/debian buster-updates main non-free
addrepo "$buildroot" sources.list http://security.debian.org/ buster/updates main non-free

# add BSP repos
addrepo "$buildroot" sources.list.d/solidrun.list https://repo.solid-build.xyz/debian/buster/bsp-any ./
addrepo "$buildroot" sources.list.d/solidrun.list https://repo.solid-build.xyz/debian/buster/bsp-imx6 ./
installpkg_url "$buildroot" https://repo.solid-build.xyz/debian/buster/bsp-any/all/solidrun-keyring_2019.10.11_all.deb

# mount /dev/root from fstab
# Note: breaks flash-kernel package which expects a more meaningful value to include in initrd!
# Note: replaced with uuid by mkimage
cat > "$buildroot/etc/fstab" << EOF
/dev/root / auto defaults 0 0
EOF

# refresh repos
chroot_run "$buildroot" apt-get update

# apply security updates missed by debootstrap
update "$buildroot"

# install HW support packages
installpkg "$buildroot" bsp-solidrun-imx6 linux-image-4.9.y-imx6-sr

# install a bunch of useful applications
installpkg "$buildroot" sudo openssh-server connman nano psmisc ntfs-3g iw rfkill devmem2 usbutils pciutils i2c-tools mtd-utils command-not-found alsa-utils wget
chroot_run "$buildroot" apt-file update
chroot_run "$buildroot" update-command-not-found

# generate additional entropy to speed up /dev/urandom and reduce boot time
installpkg "$buildroot" haveged

# install gpu userspace
installpkg "$buildroot" imx-gpu-viv imx-gpu-viv-fb eglinfo-fb

# install multimedia userspace
installpkg "$buildroot" gstreamer1.0-imx-fb gstreamer1.0-plugins-good gstreamer1.0-plugins-bad gstreamer1.0-plugins-ugly gstreamer1.0-alsa gstreamer1.0-tools

if [ "x$flavour" == "xx11" ]; then
	installpkg "$buildroot" --no-install-recommends imx-gpu-viv-x11 eglinfo-x11 gstreamer1.0-imx-x11 mesa-utils-extra
	chroot_run "$buildroot" tasksel install desktop xfce-desktop
fi

# install automatic filesystem expander script
installpkg "$buildroot" expand-fs runonce parted
install -v -m 700 -o root -g root data/expand-rootfs.sh "$buildroot/etc/runonce.d/"

# for security reasons recreate the ssh keys on first boot
rm -fv "$buildroot"/etc/ssh/*_key{,.pub}
chroot_run "$buildroot" runonce-helper add generate-ssh-keys /usr/sbin/dpkg-reconfigure openssh-server

# add default user
adduser "$buildroot" debian debian /bin/bash audio sudo video

# set hostname
sethostname "$buildroot" sr-imx6

# set default runlevel
if [ "x$flavour" == "xx11" ]; then
	chroot_run "$buildroot" systemctl set-default graphical.target
else
	chroot_run "$buildroot" systemctl set-default multi-user.target
fi

# spawn getty on serial port
chroot_run "$buildroot" systemctl enable serial-getty@ttymxc0.service

# enable watchdog
sed -E -i "s;^#?RuntimeWatchdogSec=.*$;RuntimeWatchdogSec=60;g" "$buildroot/etc/systemd/system.conf"

# make bootable (using debian flash-image)
installpkg "$buildroot" flash-kernel
sed -E -i "s;^LINUX_KERNEL_CMDLINE=.*$;LINUX_KERNEL_CMDLINE=\"log_level=7 net.ifnames=0\";g" "$buildroot/etc/default/flash-kernel"
for dev in \
	"SolidRun Cubox-i Dual/Quad" \
	"SolidRun Cubox-i Dual/Quad (1.5som)" \
	"SolidRun Cubox-i Dual/Quad (1.5som+emmc)" \
	"SolidRun Cubox-i Solo/DualLite" \
	"SolidRun Cubox-i Solo/DualLite (1.5som)" \
	"SolidRun Cubox-i Solo/DualLite (1.5som+emmc)" \
	"SolidRun HummingBoard2 Dual/Quad" \
	"SolidRun HummingBoard2 Dual/Quad (1.5som)" \
	"SolidRun HummingBoard2 Dual/Quad (1.5som+emmc)" \
	"SolidRun HummingBoard2 Solo/DualLite" \
	"SolidRun HummingBoard2 Solo/DualLite (1.5som)" \
	"SolidRun HummingBoard2 Solo/DualLite (1.5som+emmc)" \
	"SolidRun HummingBoard Dual/Quad" \
	"SolidRun HummingBoard Dual/Quad (1.5som)" \
	"SolidRun HummingBoard Dual/Quad (1.5som+emmc)" \
	"SolidRun HummingBoard Solo/DualLite" \
	"SolidRun HummingBoard Solo/DualLite (1.5som)" \
	"SolidRun HummingBoard Solo/DualLite (1.5som+emmc)" \
	; do
	chroot_run "$buildroot" env FK_MACHINE=\"$dev\" flash-kernel
done

# remove traces of build-system
cleanup "$buildroot"
