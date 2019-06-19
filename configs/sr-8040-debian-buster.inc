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
export FK_MACHINE=none
# bootstrap a debian system
bootstrap "$buildroot" buster arm64 apt-transport-https ca-certificates locales systemd-sysv initramfs-tools busybox

# add debian repos
deleterepos "$buildroot" sources.list
addrepo "$buildroot" sources.list http://httpredir.debian.org/debian buster main non-free
addrepo "$buildroot" sources.list http://httpredir.debian.org/debian buster-updates main non-free
addrepo "$buildroot" sources.list http://security.debian.org/ buster/updates main non-free

# add BSP repo
addrepo "$buildroot" sources.list.d/solidrun.list https://repo.solid-build.xyz/debian/buster/bsp-8040 ./
installpkg_url "$buildroot" https://repo.solid-build.xyz/debian/buster/bsp-8040/all/solidrun-keyring_2019.02.18_all.deb

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
installpkg "$buildroot" flash-kernel runonce expand-fs parted linux-image-arm64 firmware-atheros bsp-solidrun-8040
install -v -m700 -o root -g root data/expand-rootfs.sh "$buildroot/etc/runonce.d/"
install -v -m644 -o root -g root /home/josua/Downloads/armada-8040-clearfog-gt-8k.dtb "$buildroot/etc/flash-kernel/dtbs/

# install a bunch of useful applications
installpkg "$buildroot" sudo openssh-server nano psmisc ntfs-3g iw rfkill usbutils pciutils i2c-tools mtd-utils command-not-found
chroot_run "$buildroot" apt-file update
chroot_run "$buildroot" update-command-not-found

# for security reasons recreate the ssh keys on first boot
rm -fv "$buildroot"/etc/ssh/*_key{,.pub}
chroot_run "$buildroot" runonce-helper add generate-ssh-keys /usr/sbin/dpkg-reconfigure openssh-server

# add default user
adduser "$buildroot" debian debian /bin/bash sudo

# set hostname
sethostname "$buildroot" sr-8040

# set default runlevel
chroot_run "$buildroot" systemctl set-default multi-user.target

# spawn getty on serial port
chroot_run "$buildroot" systemctl enable serial-getty@ttyS0.service

# configure network
install -v -m 644 -o root -g root data/interface-eth0 "$buildroot/etc/network/interfaces.d/eth0"
install -v -m 644 -o root -g root data/interface-eth1 "$buildroot/etc/network/interfaces.d/eth1"

# enable watchdog
sed -i "s;^#?RuntimeWatchdogSec=.*;RuntimeWatchdogSec=60;g" "$buildroot/etc/systemd/system.conf"

# make bootable
sed -i "s;quiet;log_level=7 net.ifnames=0;g" "$buildroot/etc/default/flash-kernel"
chroot_run "$buildroot" env FK_MACHINE=\"SolidRun ClearFog GT 8K\" flash-kernel
chroot_run "$buildroot" env FK_MACHINE=\"Marvell 8040 MACCHIATOBin Double-shot\" flash-kernel

# remove traces of build-system
cleanup "$buildroot"
