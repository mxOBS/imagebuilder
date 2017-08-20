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
		umount ${LODEV}p2 || true
		rmdir $MOUNT/boot/efi || true
		rmdir $MOUNT/boot || true
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

# create partitions
# tiny EFI partition at 2048-16383
# rootfs beyond (so it starts at 8192)
printf "Creating partition table: "
echo "o
n
p
1

16383
n
p
2


t
1
ef
w
q" | fdisk $LODEV 1>/dev/null 2>/dev/null
printf "Done\n"

# reload partition table
partprobe $LODEV

# create filesystems
printf "Creating new EFI filesystem: "
mkfs.fat ${LODEV}p1 1>/dev/null 2>/dev/null
test $? != 0 && printf "Failed\n" && exit 1
printf "Done\n"

printf "Creating new ext4 filesystem: "
mkfs.ext4 -L rootfs ${LODEV}p2 1>/dev/null 2>/dev/null
test $? != 0 && printf "Failed\n" && exit 1
printf "Done\n"

# mount root filesystem
MOUNT=linux
mkdir $MOUNT
mount ${LODEV}p2 $MOUNT

# mount efi filesystem
mkdir -p $MOUNT/boot/efi
mount ${LODEV}p1 $MOUNT/boot/efi

# install files
printf "Unpacking rootfs into image: "
tar -C $MOUNT --numeric-owner -xpf $archive
test $? != 0 && exit 1
printf "Done\n"

# find efi filesystem uuid
EFIUUID=$(lsblk -n -o UUID ${LODEV}p1)

# find root filesystem uuid
ROOTUUID=$(lsblk -n -o UUID ${LODEV}p2)

# patch fstab replacing generic /dev/root name if any
printf "Patching fstab with actual filesystem UUIDs: "
sed -i "s;^/dev/root;UUID=$ROOTUUID;g" $MOUNT/etc/fstab
test $? != 0 && exit 1
# add mountpoint for EFI partition to fstab
cat >> $MOUNT/etc/fstab << EOF
UUID=$EFIUUID /boot/efi vfat umask=0002 0 0
EOF
test $? != 0 && exit 1
printf "Done\n"

# install a firstboot grub efi image
printf "Installing initial grub image: "
FILE=$(mktemp)
cat > $FILE << EOF
set timeout=3
set default=0
menuentry "Default Kernel" {
	search --no-floppy --fs-uuid --set root $ROOTUUID
	linux /boot/Image root=UUID=$ROOTUUID rootwait console=ttyS0,115200n8
	initrd /boot/initrd
}
EOF
grub2-mkstandalone \
	--directory=$MOUNT/usr/lib/grub/arm64-efi \
	--locale-directory=$MOUNT/usr/share/locale \
	-O arm64-efi \
	--modules="acpi fdt part_msdos part_gpt fat ext2 search_fs_uuid" \
	-o $MOUNT/boot/efi/efi/boot/bootaa64.efi \
	"boot/grub/grub.cfg=$FILE"
s=$?
rm -f $FILE
test $s != 0 && exit 1
printf "Done\n"
s=

# flush caches
printf "Flushing kernel filesystem caches: "
sync
printf "Done\n"

# umount
umount ${LODEV}p1
umount ${LODEV}p2
rmdir $MOUNT
MOUNT=

# detach loopback device
losetup -d $LODEV
LODEV=
IMG=

# done
echo "Finished creating image."
exit 0
