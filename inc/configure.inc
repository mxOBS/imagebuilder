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
	echo "auto eth0" >> $buildroot/etc/network/interfaces
	echo "iface eth0 inet dhcp" >> $buildroot/etc/network/interfaces
	echo "" >> $buildroot/etc/network/interfaces
	echo "# To find out how to properly use this file, see the examples at /usr/share/doc/ifupdown/examples/network-interfaces.gz!" >> $buildroot/etc/network/interfaces
	echo "# gzip -dck /usr/share/doc/ifupdown/examples/network-interfaces.gz | less" >> $buildroot/etc/network/interfaces

	# autogenerate ssh-keys on first boot, and delete the current ones
	rm -fv $buildroot/etc/ssh/*_key{,.pub}
	chroot_run $buildroot runonce-helper add generate-ssh-keys /usr/sbin/dpkg-reconfigure openssh-server

	# create default user
	user=solidrun
	password=solidrun
	groups="sudo,audio,video"
	password_encrypted=`perl -e 'print crypt($ARGV[0], "password")' $password`
	chroot_run $buildroot useradd -G $groups -m -U -s /bin/bash -p $password_encrypted $user

	# enable lightdm autologin
	if [ -e "$buildroot/etc/lightdm/lightdm.conf" ]; then
		sed -e "s;#autologin-user-timeout=0;autologin-user-timeout=5;g" \
		    -e "s;#autologin-user=;autologin-user=$user;g" \
		    -i $buildroot/etc/lightdm/lightdm.conf
	fi
}
