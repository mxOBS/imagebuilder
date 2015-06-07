# usage: <buildroot>
cleanup_system() {
	# clean apt package store
	chroot_run $buildroot "apt-get clean"

	# remove resolv.conf of host system
	rm -vf $buildroot/etc/resolv.conf

	# delete ssh-keys; it is very insecure to have them pregenerated!
	rm -fv $buildroot/etc/ssh/*_key{,.pub}
}
