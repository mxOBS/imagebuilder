#!/bin/bash -e

usage() {
	echo "$0 <device> <tarball>"
	exit 1
}

if [ "x$#" != "x2" ]; then
	usage
fi
device="$1"
tarball="$2"

if [ "x$UID" != "x0" ]; then
	echo Error: This script requires root privileges!
	exit 1
fi

# cleanup on unexpected exit
cleanup() {
	if [ -d "mnt" ]; then
		umount mnt || true
	fi
}
trap cleanup 0

# partition disk
fdisk "$device" << EOF
o
n
p
1


w
q
EOF
partprobe "$device"

# create filesystem
mkfs.ext4 -L "rootfs" "${device}1"

# mount filesystem
mkdir -p mnt
mount "${device}1" mnt

# unpack tarball
tar --numeric-owner -C mnt -xvpf "$tarball"

# install bootcode
dd if=mnt/boot/cubox-i-spl.bin of="$device" bs=1K seek=1
dd if=mnt/boot/u-boot.img of="$device" bs=1K seek=42

# umount filesystem
sync
umount mnt
rmdir mnt

# End
echo Done
