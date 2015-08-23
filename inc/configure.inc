# usage: <buildroot>
configure_system() {
	buildroot=$1

	# set hostname
	hostname=linux
	# debootstrap set the current systems hostname, replace it
	sudo sed -i "s;$HOSTNAME;$hostname;g" $buildroot/etc/{hostname,ssh/*.pub}
	# set hostname in hosts file
	echo "127.0.0.2 $hostname" >> $buildroot/etc/hosts

	# spawn getty on serial port
	if [ -e "$buildroot/etc/inittab" ]; then
		#sysvinit
		echo "# i.MX serial console" >> "$buildroot/etc/inittab"
		echo "T0:23:respawn:/sbin/getty -L ttymxc0 115200 vt100" >> "$buildroot/etc/inittab"
	else
		# systemd
		chroot_run $buildroot systemctl enable serial-getty@ttymxc0.service
	fi

	# ifup/down config file; connman is used, so no configuration done here
	cat > $buildroot/etc/network/interfaces << EOF
# interfaces(5) file used by ifup(8) and ifdown(8)

# CONNMAN is installed on this system. Please dont use ifup/down.
# Instead try connman-curses or connmanctl, which are both preinstalled!

# To find out how to properly use this file, see the examples at /usr/share/doc/ifupdown/examples/network-interfaces.gz!
# gzip -dck /usr/share/doc/ifupdown/examples/network-interfaces.gz | less
EOF

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

	# set default timezone UTC
	echo "Etc/UTC" > $buildroot/etc/timezone
	cp -f $buildroot/usr/share/zoneinfo/Etc/UTC $buildroot/etc/localtime
	# TODO: update debconf database

	# set default locale
	sed -i "s;# en_US.UTF-8 UTF-8;en_US.UTF-8 UTF-8;g" $buildroot/etc/locale.gen
	chroot_run $buildroot /usr/sbin/locale-gen
	chroot_run $buildroot /usr/sbin/update-locale --no-checks LANG
	chroot_run $buildroot /usr/sbin/update-locale "LANG=en_US.UTF-8"
	# TODO: update debconf database

	# set default keyboard layout
	sed \
		-e "s|^ *XKBMODEL=.*|XKBMODEL=\"pc105\"|" \
		-e "s|^ *XKBLAYOUT=.*|XKBLAYOUT=\"us\"|" \
		-e "s|^ *XKBVARIANT=.*|XKBVARIANT=\"intl\"|" \
		-e "s|^ *XKBOPTIONS=.*|XKBOPTIONS=\"\"|" \
		-e "s|^ *BACKSPACE=.*|BACKSPACE=\"guess\"|" \
		-i $buildroot/etc/default/keyboard
	# TODO update debconf database
}
