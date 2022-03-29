# 2022Kerrighed
Kerrrighed installation and setup guide in 2022


You will need:

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




