function qemu_setup() {
	buildroot=$1

	if [[ `uname -m` != arm* ]]; then
		cp /usr/bin/qemu-arm $buildroot/usr/bin/
	fi
}

function qemu_remove() {
	buildroot=$1

	if [[ `uname -m` != arm* ]]; then
		rm $buildroot/usr/bin/qemu-arm
	fi
}
