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
printf "Done\n"

# find filesystem uuid
UUID=$(lsblk -n -o UUID ${LODEV}p1)

# mount filesystem
MOUNT=linux
mkdir $MOUNT
mount ${LODEV}p1 linux

# install files
printf "Unpacking rootfs into image: "
tar -C $MOUNT --numeric-owner -xpf $archive
test $? != 0 && exit 1
printf "Done\n"

# install u-boot
printf "Installing bootloader: "
if [ -e $MOUNT/boot/cubox-i-spl.bin ] && [ -e $MOUNT/boot/u-boot.img ]; then
	# IMX6
	dd if=$MOUNT/boot/cubox-i-spl.bin of=$LODEV bs=1K seek=1 1>/dev/null 2>/dev/null
	dd if=$MOUNT/boot/u-boot.img of=$LODEV bs=1K seek=69 1>/dev/null 2>/dev/null
fi

# A38X - Marvell U-Boot
if [ -e $MOUNT/boot/u-boot-clearfog.mmc ]; then
	dd if=$MOUNT/boot/u-boot-clearfog.mmc of=$LODEV bs=512 seek=1 1>/dev/null 2>/dev/null
	cat > $MOUNT/boot.script << EOF
# perform first-boot tasks
if test "\${sr_firstboot}" != "done"; then
	# save initial environment
	setenv sr_firstboot done
	saveenv
fi

# configure bootargs
setenv bootargs 'root=/dev/mmcblk0p1 rootfstype=ext4 rootwait rw console=ttyS0,115200n8'

# configure addresses
kerneladdr=0x2000000
fdtaddr=0x5F00000
ramdiskaddr=0x6000000
setenv fdt_high 0x07a12000
setenv initrd_high 0xFFFFFFFF

# make sure fdt_file is set
if test "\${fdt_file}" = "\${fdt_file}"; then
	# fall back to Clearfog Pro
	fdt_file=armada-388-clearfog-pro.dtb
fi

# load DTB
echo "Loading dtb/\${fdt_file}"
ext4load mmc 0:1 \${fdtaddr} /boot/dtb/\${fdt_file}

# load Kernel
echo "Loading zImage ..."
ext4load mmc 0:1 \${kerneladdr} /boot/zImage

# load Ramdisk
echo "Loading initrd ..."
ext4load mmc 0:1 \${ramdiskaddr} /boot/initrd
ramdisksize=0x\${filesize}

# boot
echo "Booting ..."
bootz \${kerneladdr} \${ramdiskaddr}:\${ramdisksize} \${fdtaddr}
EOF
	mkimage -A arm -O linux -T script -C none -a 0 -e 0 -d $MOUNT/boot.script $MOUNT/boot.scr 1>/dev/null 2>/dev/null
fi

# A38X Mainline U-Boot with Distro support
if [ -e $MOUNT/boot/spl-clearfog.kwb ]; then
	dd if=$MOUNT/boot/spl-clearfog.kwb of=$LODEV bs=512 seek=1 1>/dev/null 2>/dev/null

	# create generic extlinux.conf pointing to standard symlinks
	# Note: sadly not debian-standard!
	install -d -o root -g root $MOUNT/boot/extlinux
	cat > $MOUNT/boot/extlinux/extlinux.conf << EOF
TIMEOUT 0
LABEL default
	LINUX ../zImage
	INITRD ../initrd
	FDTDIR ../dtb/
	APPEND console=ttyS0,115200n8 root=UUID=$UUID rootwait
EOF
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
