# This function adds all custom repositories to a system
# usage: <buildroot> <distro-codename>
add_repos() {
	buildroot=$1
	distro=$2

	# i.MX6 BSP repo
	repofile=$buildroot/etc/apt/sources.list.d/imx6.list
	prefsfile=$buildroot/etc/apt/preferences.d/imx6
	if [ "$distro" = "wheezy" ]; then
		echo "deb http://repo.jm0.eu/BSP:/Cubox-i/Debian_Wheezy/ ./" > $repofile
		echo "deb-src http://repo.jm0.eu/BSP:/Cubox-i/Debian_Wheezy/ ./" >> $repofile
		echo "Package: *" > $prefsfile
		echo "Pin: release o=obs://mx6bs/BSP:Cubox-i/Debian_Wheezy/" >> $prefsfile
		echo "Pin-Priority: 600" >> $prefsfile
		curl -L http://repo.jm0.eu/BSP:/Cubox-i/Debian_Wheezy/Release.key | chroot_run $buildroot "apt-key add -"
	fi
	if [ "$distro" = "jessie" ]; then
		echo "deb http://repo.jm0.eu/BSP:/Cubox-i/Debian_Jessie/ ./" > $repofile
		echo "deb-src http://repo.jm0.eu/BSP:/Cubox-i/Debian_Jessie/ ./" >> $repofile
		echo "Package: *" >> $prefsfile
		echo "Pin: release o=obs://mx6bs/BSP:Cubox-i/Debian_Jessie/" >> $prefsfile
		echo "Pin-Priority: 600" >> $prefsfile
		curl -L http://repo.jm0.eu/BSP:/Cubox-i/Debian_Wheezy/Release.key | chroot_run $buildroot "apt-key add -"
	fi
	if [ "$distro" = "trusty" ]; then
		echo "deb http://repo.jm0.eu/BSP:/Cubox-i/Ubuntu_Trusty_Tahr/ ./" > $repofile
		echo "deb-src http://repo.jm0.eu/BSP:/Cubox-i/Ubuntu_Trusty_Tahr/ ./" >> $repofile
		echo "Package: *" >> $prefsfile
		echo "Pin: release o=obs://mx6bs/BSP:Cubox-i/Ubuntu_Trusty_Tahr/" >> $prefsfile
		echo "Pin-Priority: 600" >> $prefsfile
		curl -L http://repo.jm0.eu/BSP:/Cubox-i/Debian_Wheezy/Release.key | chroot_run $buildroot "apt-key add -"
	fi

	# refresh apt cache
	chroot_run $buildroot "apt-get update"
}
