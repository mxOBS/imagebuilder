#!/bin/bash

# require root
if [ $UID -ne 0 ]; then
	echo "root privileges required, invoking sudo ..."
	sudo "$SHELL" "$0" $*
	exit $?
fi

# find all installed kernels
count=0
declare -a kernels
for file in $(find /boot -type f -name "zImage-*"); do
	# extract kernelrelease
	KERNELRELEASE="$(basename $file | sed -e "s;^zImage-;;g")"

	# save in array
	kernels[$count]="$KERNELRELEASE"

	# count
	((count=count+1))
done

if [ $count -le 0 ]; then
	echo "ERROR: Did not find any installed kernel!"
	exit 1
fi

# print list of installed kernels
echo "The following kernels are available on your system:"
i=0
while [ $i -lt $count ]; do
	# print kernel and id
	echo "$i: ${kernels[$i]}"

	# increment i
	((i=i+1))
done

# query choice
i=-1
read -p "Please choose a kernel from above and enter its number: " i
[ $? -ne 0 ] && exit 1

if ! [[ $i =~ ^[0-9]+$ ]] || [ $i -lt 0 ] || [ $i -ge $count ]; then
	echo "Invalid kernel selected, exiting."
	exit 1
fi

# print confirmation message
read -p "Going to select ${kernels[$i]}. Press any key to proceed, or ctrl+c to abort"
[ $? -ne 0 ] && exit 1

# create links
set -e
KERNELRELEASE="${kernels[$i]}"
ln -sfv "zImage-$KERNELRELEASE" /boot/zImage
ln -sfv "dtb-$KERNELRELEASE" /boot/dtb
ln -sfv "initrd.img-$KERNELRELEASE" /boot/initrd
echo "Done"
