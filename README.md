# SolidRun System Images Build Scripts

# Examples:

- i.MX6 Debian:

      sudo kiwi-ng system build \
      	--description ./imx6/debian-10 \
      	--target-dir ./out \
      	--signing-key=./imx6/debian-10/debian_10.key \
      	--signing-key=./imx6/debian-10/debian_10_auto.key \
      	--signing-key=./imx6/debian-10/debian_10_auto_sec.key \
      	--signing-key=./imx6/debian-10/debian_09_auto.key \
      	--signing-key=./imx6/debian-10/debian_09_auto_sec.key \
      	--signing-key=./imx6/debian-10/sr_bsp_any.key
