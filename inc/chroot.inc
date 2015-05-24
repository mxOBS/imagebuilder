# usage: <buildroot> <cmd>
chroot_run() {
	buildroot=$1
	cmd=`echo $@ | sed -e "s;^$buildroot ;;g"`

	qemu_setup $buildroot
	chroot "$buildroot" $cmd
	qemu_remove $buildroot
}
