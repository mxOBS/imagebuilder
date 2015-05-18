# usage: <buildroot>
configure_system() {
	buildroot=$1

	# set hostname
	hostname=linux
	# debootstrap set the current systems hostname, replace it
	sudo sed -i "s;$HOSTNAME;$hostname;g" build/etc/{hostname,ssh/*.pub}
	# set hostname in hosts file
	echo "127.0.0.2 $hostname" >> build/etc/hosts

	# configure network
	# default: dhcp on eth0
	# TODO: add placeholders for wlan
	echo "auto eth0" >> $buildroot/etc/network/interfaces
	echo "iface eth0 inet dhcp" >> $buildroot/etc/network/interfaces

	# create default user
	user=solidrun
	password=solidrun
	groups="sudo,audio,video"
	password_encrypted=`perl -e 'print crypt($ARGV[0], "password")' $password`
	chroot_run $buildroot useradd -G $groups -m -U -s /bin/bash -p $password_encrypted $user

	# TODO: add imx6-config startup for first login
}
