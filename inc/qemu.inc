function qemu_setup() {
	buildroot=$1

	if [[ `uname -m` != arm* ]]; then
		# some systems, e.g. debian, have just 1 binary for qemu-user:
		if [ -e /usr/bin/qemu-arm-static ]; then
			cp /usr/bin/qemu-arm-static $buildroot/usr/bin/
			return
		fi

		# others, e.g. opensuse,  use 2:
		if [ -e /usr/bin/qemu-arm-binfmt ] && [ -e /usr/bin/qemu-arm ]; then
			cp /usr/bin/qemu-arm-binfmt $buildroot/usr/bin/
			cp /usr/bin/qemu-arm $buildroot/usr/bin/
			return
		fi

		echo "WARNING: Failed to find qemu binary!"
	fi
}

function qemu_remove() {
	buildroot=$1

	if [[ `uname -m` != arm* ]]; then
		if [ -e /usr/bin/qemu-arm-static ]; then
			rm $buildroot/usr/bin/qemu-arm-static
			return
		fi
		if [ -e /usr/bin/qemu-arm-binfmt ] && [ -e /usr/bin/qemu-arm ]; then
			rm $buildroot/usr/bin/qemu-arm-binfmt
			rm $buildroot/usr/bin/qemu-arm
			return
		fi
	fi
}
