#!/bin/sh
dd if=u-boot.bin of=u-boot-new.bin bs=1K skip=1 
sudo fastboot flash uboot u-boot-new.bin
sleep 1 
sudo fastboot reboot
echo ok
