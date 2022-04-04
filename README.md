# 2022Kerrighed
Kerrrighed installation and setup guide in 2022

----Notes---- 

I do not know if anyone would want to use Kerrighed in 2022, but if you went down the same rabbit hole I did then this guide will help you. 


I have found that kerrighed will sometimes not run on virtualbox when I/O apci is enabled in virtualbox on AMD processors. It will compile, but it will kernel panic when booting.

Helpful links:

https://wiki.ubuntu.com/EasyUbuntuClustering/UbuntuKerrighedClusterGuide




----You will need----

Copy of linux kernel 2.6.20: https://mirrors.edge.kernel.org/pub/linux/kernel/v2.6/linux-2.6.20.tar.bz2

Kerrighed source code found in this respository

A copy of Ubuntu Hardy found here: https://old-releases.ubuntu.com/releases/hardy/

specifically this works: https://old-releases.ubuntu.com/releases/hardy/ubuntu-8.04-server-amd64.iso 

A copy of virtualbox


-------------------------------
Ubuntu Hardy SETUP INSTRUCTIONS
------------------------------

Boot the Ubuntu ISO in virtualbox and follow the setup instructions

Do not try to download any extra packages because the apt repository will not work

Once initial installation complete the first thing you will need to do is repair the apt sources list.

----Repair APT----

A copy of the updated sources.list file can be found here, but if you want to do it manually you will need to replace all the archive.ubuntu.com links with old-releases.ubuntu.com. 

Once that is fixed you should be able to run "apt-get update" or "aptitude update"

---Install Required Tools---

 aptitude install dhcp3-server tftpd-hpa syslinux nfs-kernel-server nfs-common debootstrap
 
 
 ---Run some Commands---
 
 1. cp /usr/lib/syslinux/pxelinux.0 /var/lib/tftpboot
 2. mkdir /var/lib/tftpboot/pxelinux.cfg
 3. cp /boot/vmlinuz-<KERNEL_VERSION> /boot/initrd.img-<KERNEL_VERSION> /var/lib/tftpboot/
 
 
 
 
--Configure Services---

copies of the files that need to be edited are included in this repo under "config files"
edit them when appropriate because they may not work for all builds

Edit the files: 

1. /etc/default/dhcp3-server
2. /etc/dhcp3/dhcpd.conf
3. /etc/default/tftp-hpa
4. /etc/exports
5. /var/lib/tftpboot/pxelinux.cfg/default



---Configure Chroot----



1. debootstrap --arch amd64 hardy /nfsroot/kerrighed http://old-releases.ubuntu.com/ubuntu/
2. chroot /nfsroot/kerrighed
3. passwd
4. mount -t proc none /proc

now edit the sources.list to be like the one on the host. 

5. aptitude update
6. apt-get install dhcp3-common nfs-common nfsbooted openssh-server


edit the files in "chroot config files"

7. ln -sf /etc/network/if-up.d/mountnfs /etc/rcS.d/S34mountnfs 
8. adduser "username"
 
----Setup Kerrighed----


1. apt-get install automake autoconf libtool pkg-config gawk rsync bzip2 gcc-3.3 libncurses5 libncurses5-dev wget lsb-release xmlto patchutils xutils-dev build-essential openssh-server ntp
2. copy kernel and kerrighed source into /usr/src
3.  cd /usr/src
4.  tar zxf kerrighed-2.4.1.tar.gz
5.  tar jxf linux-2.6.20.tar.bz2

6. cd /usr/src/kerrighed-VERSION/modules
7. ./configure --with-kernel=/usr/src/linux-2.6.20 CC=gcc-3.3
8. cd kernel
9. make defconfig
10. make menuconfig
11. make kernel
12.  make
13.  make kernel-install
14.  make install
15.  ldconfig
16.  mkdir /config
17. cp /nfsroot/kerrighed/boot/vmlinuz-2.6.20-krg /var/lib/tftpboot/
18. exit
19. reboot





----Testing setup----

Everything should work now


commands that should work

1.krgadm cluster start
2. krgadm cluster status
3. free -m should show combined ram



