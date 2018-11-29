# 
# Copyright (c) 2017-2018 Josua Mayer
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
bootstrap "$buildroot" stretch arm64 apt-transport-https ca-certificates locales systemd-sysv initramfs-tools busybox

# add debian repos
deleterepos "$buildroot" sources.list
addrepo "$buildroot" sources.list http://httpredir.debian.org/debian stretch main non-free
addrepo "$buildroot" sources.list http://httpredir.debian.org/debian stretch-updates main non-free
addrepo "$buildroot" sources.list http://security.debian.org/ stretch/updates main non-free
# backports
addrepo "$buildroot" sources.list http://ftp.debian.org/debian stretch-backports main non-free

# add BSP repo
addrepo "$buildroot" sources.list.d/solidrun.list https://repo.solid-build.xyz/debian/stretch/bsp-imx8 ./
installpkg_url "$buildroot" https://repo.solid-build.xyz/debian/stretch/bsp-imx8/all/solidrun-keyring_2018.10.10_all.deb

# full fstab with uuid and filesystem already decided
FSUUID=$(uuidgen)
cat > "$buildroot/etc/fstab" << EOF
UUID=$FSUUID / ext4 defaults 0 0
EOF

# refresh repos
chroot_run "$buildroot" apt-get update

# apply security updates missed by debootstrap
update "$buildroot"

# install HW support packages
installpkg "$buildroot" flash-kernel linux-image-4.9.y-imx8-sr firmware-atheros bsp-solidrun-imx8
install -v -m644 -o root -g root scripts/flash-kernel.db "$buildroot/etc/flash-kernel/db"

# make bootable
sed -i "s;quiet;log_level=7;g" "$buildroot/etc/default/flash-kernel"
chroot_run "$buildroot" env FK_MACHINE=\"SolidRun i.MX8MQ HummingBoard Pulse\" flash-kernel

# install a bunch of useful applications
installpkg "$buildroot" sudo openssh-server nano psmisc ntfs-3g iw rfkill usbutils pciutils i2c-tools mtd-utils command-not-found
chroot_run "$buildroot" apt-file update
chroot_run "$buildroot" update-command-not-found

# for security reasons recreate the ssh keys on first boot
rm -fv "$buildroot"/etc/ssh/*_key{,.pub}
chroot_run "$buildroot" runonce-helper add generate-ssh-keys /usr/sbin/dpkg-reconfigure openssh-server

# add default user
adduser "$buildroot" debian debian /bin/bash video sudo

# set hostname
sethostname "$buildroot" sr-imx8

# set default runlevel
chroot_run "$buildroot" systemctl set-default multi-user.target

# spawn getty on serial port
chroot_run "$buildroot" systemctl enable serial-getty@ttymxc0.service

# configure network
install -v -m 644 -o root -g root scripts/interface-eth0 "$buildroot/etc/network/interfaces.d/eth0"

# remove traces of build-system
cleanup "$buildroot"
