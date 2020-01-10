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
addrepo "$buildroot" sources.list http://deb.debian.org/debian buster-backports main non-free

# add BSP repo
addrepo "$buildroot" sources.list.d/solidrun.list https://repo.solid-build.xyz/debian/buster/bsp-any ./
addrepo "$buildroot" sources.list.d/solidrun.list https://repo.solid-build.xyz/debian/buster/bsp-lx2k ./
installpkg_url "$buildroot" https://repo.solid-build.xyz/debian/buster/bsp-any/all/solidrun-keyring_2020.01.09_all.deb

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
installpkg "$buildroot" flash-kernel runonce expand-fs parted bsp-solidrun-lx2k linux-image-4.19.y-lx2k-sr
install -v -m700 -o root -g root data/expand-rootfs.sh "$buildroot/etc/runonce.d/"

# install a basic debian server
chroot_run "$buildroot" tasksel install basic ssh-server
installpkg "$buildroot" sudo pciutils usbutils wget psmisc rfkill i2c-tools ethtool

# for security reasons recreate the ssh keys on first boot
rm -fv "$buildroot"/etc/ssh/*_key{,.pub}
chroot_run "$buildroot" runonce-helper add generate-ssh-keys /usr/sbin/dpkg-reconfigure openssh-server

# add default user
adduser "$buildroot" debian debian /bin/bash sudo

# set hostname
sethostname "$buildroot" sr-lx2k

# set default runlevel
chroot_run "$buildroot" systemctl set-default multi-user.target

# spawn getty on serial port
chroot_run "$buildroot" systemctl enable serial-getty@ttyAMA0.service

# configure network
install -v -m 644 -o root -g root data/interface-eth0 "$buildroot/etc/network/interfaces.d/eth0"

# enable watchdog
sed -i "s;^#?RuntimeWatchdogSec=.*;RuntimeWatchdogSec=60;g" "$buildroot/etc/systemd/system.conf"

# prepare for U-Boot
sed -i "s;quiet;log_level=7 net.ifnames=0;g" "$buildroot/etc/default/flash-kernel"
chroot_run "$buildroot" env FK_MACHINE=\"SolidRun LX2160A COM express type 7 module\" flash-kernel

# prepare for UEFI (GRUB)
# create initial grub.cfg (will be rewritten on first boot)
installpkg "$buildroot" grub-efi-arm64
KVERSION=$(cd "$buildroot"; ls boot/vmlinuz-* | cut -d- -f1 --complement)
mkdir -p "$buildroot/boot/efi/EFI/BOOT"
cat > "$buildroot/boot/grub/grub.cfg" << EOF
set timeout=5
set default=0

menuentry "Debian for SolidRun LX2K devices - First-Boot Stage" {
	search --no-floppy --fs-uuid --set=root $FSUUID
	linux /boot/vmlinuz-$KVERSION root=UUID=$FSUUID
	initrd /boot/initrd.img-$KVERSION
}
EOF
install -v -m700 -o root -g root data/grub-config.sh "$buildroot/etc/runonce.d/"

# Create U-Boot Script on EFI partition for redirecting to real boot script
# avoids running grub.efi from u-boot
cat > "$buildroot/boot/efi/boot.txt" << EOF
# redirect to second partition
setenv distro_bootpart 2
setenv prefix boot/
setenv script boot.scr
load \${devtype} \${devnum}:\${distro_bootpart} \${scriptaddr} \${prefix}\${script}
source \${scriptaddr}
EOF
chroot_run "$buildroot" mkimage -A arm64 -O linux -T script -C none -a 0 -e 0 -d /boot/efi/boot.txt /boot/efi/boot.scr

# remove traces of build-system
cleanup "$buildroot"
