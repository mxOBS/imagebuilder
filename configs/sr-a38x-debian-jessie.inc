# 
# Copyright (c) 2015-2017 Josua Mayer
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
bootstrap "$buildroot" jessie armhf apt-transport-https ca-certificates initramfs-tools locales

# Configure Defaults (before any packages, so they are taken into account by post scripts)
# enable fsck on every boot
sed -i "s;^.*FSCKFIX=.*$;FSCKFIX=yes;g" /etc/default/rcS

# add debian repos
deleterepos "$buildroot" sources.list
addrepo "$buildroot" sources.list http://httpredir.debian.org/debian jessie main
addrepo "$buildroot" sources.list http://httpredir.debian.org/debian jessie-updates main
addrepo "$buildroot" sources.list http://security.debian.org/ jessie/updates main

# add BSP repo
addrepo "$buildroot" sources.list.d/solidrun.list https://repo.solid-build.xyz/debian/jessie/bsp-imx6 ./
addaptkey "$buildroot" https://repo.solid-build.xyz/debian/jessie/bsp-imx6/Release.key

# refresh repos
chroot_run "$buildroot" apt-get update

# install HW support packages
installpkg "$buildroot" u-boot-clearfog linux-image-4.9.y-clearfog

# install a bunch of useful applications
installpkg "$buildroot" sudo openssh-server connman connman-curses-git nano psmisc ntfs-3g iw rfkill
installpkg "$buildroot" devmem2 usbutils pciutils i2c-tools mtd-utils
installpkg "$buildroot" command-not-found
chroot_run "$buildroot" update-command-not-found

# install automatic filesystem expander script
installpkg "$buildroot" expand-fs runonce
cat > "$buildroot/etc/runonce.d/expand-fs.sh" << EOF
# run resize script
/bin/bash /usr/sbin/expand-fs /dev/mmcblk0 /dev/mmcblk0p1
s=\$?

# check exit status
if [ \$s = 2 ]; then
	# partprobe failed, so schedule stage 2 once more
	runonce-helper add expand-fs-stage2 /bin/bash /usr/sbin/expand-fs /dev/mmcblk0 /dev/mmcblk0p1 2
	exit 0
fi

# return exit status
exit \$s
EOF

# install helper script for easy boot kernel selection
install -v -m755 -o root -g root scripts/selectkernel.sh "$buildroot/boot/"

# for security reasons recreate the ssh keys on first boot
rm -fv "$buildroot"/etc/ssh/*_key{,.pub}
chroot_run "$buildroot" runonce-helper add generate-ssh-keys /usr/sbin/dpkg-reconfigure openssh-server

# add default user
adduser "$buildroot" debian debian /bin/bash sudo

# set hostname
sethostname "$buildroot" clearfog

# set default runlevel
chroot_run "$buildroot" systemctl set-default multi-user.target

# spawn getty on serial port
chroot_run "$buildroot" systemctl enable serial-getty@ttyS0.service

# mount /dev/root from fstab
# Note: breaks flash-kernel package which expects a more meaningful value to include in initrd!
cat > "$buildroot/etc/fstab" << EOF
/dev/root / auto defaults 0 0
EOF

# remove traces of build-system
cleanup "$buildroot"
