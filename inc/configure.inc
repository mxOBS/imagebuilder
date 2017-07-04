# 
# Copyright (c) 2015 Josua Mayer
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# 

# usage: <buildroot> <distro codename>
configure_system() {
	local buildroot=$1
	local distro=$2

	# set hostname
	local hostname=linux
	# debootstrap set the current systems hostname, replace it
	sudo sed -i "s;$HOSTNAME;$hostname;g" $buildroot/etc/{ssh/*.pub}
	echo "$hostname" > $buildroot/etc/hostname
	# set hostname in hosts file
	echo "127.0.0.2 $hostname" >> $buildroot/etc/hosts

	# spawn getty on serial port
	if [ -e "$buildroot/etc/inittab" ]; then
		#sysvinit
		echo "# i.MX serial console" >> "$buildroot/etc/inittab"
		echo "T0:23:respawn:/sbin/getty -L ttymxc0 115200 vt100" >> "$buildroot/etc/inittab"
	else
	if [ -e "$buildroot/bin/systemctl" ] || [ -e "$buildroot/usr/bin/systemctl" ]; then
		# systemd
		chroot_run $buildroot systemctl enable serial-getty@ttymxc0.service
	else
	if [ -d "$buildroot/etc/init" ]; then
		# upstart
		echo "NOPE"
	fi; fi; fi

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
	local user=debian
	local password=debian
	local groups="sudo,audio,video"
	local password_encrypted=`perl -e 'print crypt($ARGV[0], "password")' $password`
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

	# HACK: set sticky-bit on ping and ping6
	chroot_run $buildroot chmod +s /bin/ping /bin/ping6

	# HACK: unblock wifi on every boot unconditionally
	# inspired by mk01, thanks a lot!
	# TODO: figure out how to get rid of the call to sleep
	echo "KERNEL==\"wlan*\", RUN+=\"/bin/sh -c '/bin/mknod /dev/rfkill c 10 62; sleep 1; /usr/sbin/rfkill unblock wifi'\"" \
	> $buildroot/etc/udev/rules.d/unblock-wifi.rules
}
