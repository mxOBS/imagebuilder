#!/bin/bash
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

# args
if [ $# != 2 ]; then
	echo "Usage: $0 <system-archive> <size_in_megabytes>"
	exit 1
fi
archive=$1
size=$2

# check system
if [ "$UID" != 0 ]; then
	echo "root privileges required!"
	exit 1
fi

progs="dd losetup fdisk tar mkfs.ext4 qemu-img mkimage partprobe"
for prog in $progs; do
	which $prog 1>/dev/null 2>/dev/null
	if [ $? != 0 ]; then
		echo "Missing $prog!"
		exit 1
	fi
done

# initialize temporary variables
IMG=
LODEV=
MOUNT=

# create exit handler
function cleanup() {
	if [ ! -z "$MOUNT" ]; then
		umount ${LODEV}p1 || true
		rmdir $MOUNT || true
	fi
	if [ ! -z "$LODEV" ]; then
		losetup -d $LODEV || true
	fi
	if [ ! -z "$IMG" ] && [ -e "$IMG" ]; then
		rm -f "$IMG" || true
	fi
#	echo cleaned up
}
trap cleanup INT TERM EXIT

# create image file
IMG=$(echo $archive | cut -d. -f1).img
printf "Creating %s with a size of %s: " $IMG $size
qemu-img create "$IMG" $size 1>/dev/null
printf "Done\n"

# attach image to loopback device
printf "Attaching image to unused loopback device: "
LODEV=`losetup -f`
losetup $LODEV "$IMG"
test $? != 0 && exit 1
printf "Done\n"

# create partition
# optimize for sdcards, and start at block 8192
printf "Creating partition table: "
echo "o
n
p
1
8192

w
q" | fdisk $LODEV 1>/dev/null 2>/dev/null
printf "Done\n"

# reload partition table
partprobe $LODEV

# create filesystem
printf "Creating new ext4 filesystem: "
mkfs.ext4 -L rootfs ${LODEV}p1 1>/dev/null 2>/dev/null
test $? != 0 && printf "Failed\n" && exit 1
FS=ext4
printf "Done\n"

# mount filesystem
MOUNT=linux
mkdir $MOUNT
mount ${LODEV}p1 linux

# install files
printf "Unpacking rootfs into image: "
tar -C $MOUNT --numeric-owner -xpf $archive
test $? != 0 && exit 1
printf "Done\n"

# find filesystem uuid
UUID=$(lsblk -n -o UUID ${LODEV}p1)

# patch fstab replacing generic /dev/root name if any
printf "Patching fstab with actual filesystem UUID: "
sed -i "s;^/dev/root;UUID=$UUID;g" $MOUNT/etc/fstab
test $? != 0 && exit 1
printf "Done\n"

# install u-boot
printf "Installing bootloader: "
loader_installed=no
if [ -e $MOUNT/boot/cubox-i-spl.bin ] && [ -e $MOUNT/boot/u-boot.img ]; then
	# IMX6
	dd if=$MOUNT/boot/cubox-i-spl.bin of=$LODEV bs=1K seek=1 1>/dev/null 2>/dev/null
	dd if=$MOUNT/boot/u-boot.img of=$LODEV bs=1K seek=69 1>/dev/null 2>/dev/null

	# This is the new U-Boot :) use extlinux.conf
	# create generic extlinux.conf pointing to standard symlinks
	# Note: sadly not debian-standard!
	install -d -o root -g root $MOUNT/boot/extlinux
	cat > $MOUNT/boot/extlinux/extlinux.conf << EOF
TIMEOUT 0
LABEL default
	LINUX ../zImage
	INITRD ../initrd
	FDTDIR ../dtb-dir/
	APPEND console=ttymxc0,115200n8 root=UUID=$UUID rootfstype=$FS rootwait
EOF
	loader_installed=yes
fi

# GTA04
if [ -h $MOUNT/boot/uImage ] && [[ $(readlink $MOUNT/boot/uImage) = uImage-*-letux ]]; then
	# boot script because original one does not look for DTB in /boot/dtb/
	cat > $MOUNT/boot/boot.script << EOF
i2c dev 0
mmc rescan 0
ext4load mmc 0:1 \${loadaddr} /boot/uImage
ext4load mmc 0:1 \${loadaddrfdt} /boot/dtb/\${devicetree}.dtb 
# TODO: initrd
setenv bootargs console=ttyO2,115200n8 root=/dev/mmcblk0p1 rootfstype=ext4 rootwait
bootm \${loadaddr} - \${loadaddrfdt}
EOF
    mkimage -A arm -O linux -T script -C none -a 0 -e 0 -d $MOUNT/boot/boot.script $MOUNT/boot/bootargs.scr 1>/dev/null 2>/dev/null
fi

# Generic (zImage - not Debian)
if [ "x$loader_installed" != "xyes" ]; then
	mkdir -p $MOUNT/boot/extlinux
	cat > $MOUNT/boot/extlinux/extlinux.conf << EOF
TIMEOUT 0
LABEL default
	LINUX ../zImage
	INITRD ../initrd
	FDTDIR ../dtb-dir/
	APPEND root=UUID=$UUID rootfstype=auto rootwait
EOF

	loader_installed=yes
fi

printf "Done\n"

# flush caches
printf "Flushing kernel filesystem caches: "
sync
printf "Done\n"

# umount
umount ${LODEV}p1
rmdir $MOUNT
MOUNT=

# detach loopback device
losetup -d $LODEV
LODEV=
IMG=

# done
echo "Finished creating image."
exit 0
