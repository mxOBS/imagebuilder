precheck() {
	# make sure the build-area is clean
	if [ -e "build" ]; then
		echo Error: a file or directory called \"build\" exists already!
		exit 1
	fi

	if [ "x$UID" != "x0" ]; then
		echo Error: This script requires root privileges!
		exit 1
	fi
}

chroot_run() {
	cmd="$@"
	chroot "build" $cmd
}

fetch() {
	if [ ! -e "`basename $1`" ]; then
		wget "$1"
	fi
}

chroot_add_user() {
	user="$1"
	pass="$2"
	groups="$3"
	encpass=`perl -e 'print crypt($ARGV[0], "password")' $pass`
	chroot_run useradd -G $groups -m -U -s /bin/bash -p $encpass $user
}
