# 2022Kerrighed
Kerrrighed installation and setup guide in 2022

----Notes---- 

I do not know if anyone would want to use Kerrighed in 2022, but if you went down the same rabbit hole I did then this guide will help you. 
I have found

I have found that kerrighed will sometimes not run on virtualbox when I/O apci is enabled in virtualbox on AMD processors. It will compile, but it will give a CPU timeout error when booting.  





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

copies of the files that need to be edited are included in this repo
edit them when appropriate because they may not work for all builds

Edit the files: 

1. /etc/default/dhcp3-server
2. /etc/dhcp3/dhcpd.conf
3. /etc/default/tftp-hpa
4. /etc/exports
5. /var/lib/tftpboot/pxelinux.cfg/default


---Configure Chroot----











