# usage: <buildroot> <cmd>
chroot_run() {
	buildroot=$1
	cmd=`echo $@ | sed -e "s;^$buildroot ;;g"`
	chroot "$buildroot" $cmd
}
