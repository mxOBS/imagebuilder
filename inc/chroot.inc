# usage: <buildroot> <cmd>
chroot_run() {
	buildroot=$1
	shift
	cmd="$@"

	qemu_setup $buildroot
	echo $cmd
	chroot "$buildroot" $cmd
	qemu_remove $buildroot
}
