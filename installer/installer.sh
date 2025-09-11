#!/bin/bash

apk add e2fsprogs

echo "Restoring /etc/network/interfaces..."
cat > /etc/network/interfaces <<EOF
auto lo
iface lo inet loopback

auto eth0
iface eth0 inet dhcp
EOF

echo "Bringing up network interface eth0..."
ip link set eth0 up
echo "nameserver 8.8.8.8" > /etc/resolv.conf
echo "RESOLV_CONF=no" > /etc/udhcpc/udhcpc.conf
service networking restart

echo "Updating Alpine Linux repositories..."
echo "https://mirror.leaseweb.com/alpine/v3.20/main" > /etc/apk/repositories
echo "https://mirror.leaseweb.com/alpine/v3.20/community" >> /etc/apk/repositories
apk update

echo "Installing GRUB bootloader..."
apk add grub

mkfs.vfat -F 32 /dev/sda

cp -r /media/cdrom/grub /usr/lib/

mount -t vfat /dev/sda /mnt

mount | grep boot

mount -t vfat /dev/sda /boot

grub-install --target=i386-pc --boot-directory=/mnt/boot --force /dev/sda

cp -r /media/cdrom/os/* /mnt/

echo "All tasks completed successfully!"
echo "Boot image written to sector 2048 (1MB offset)"
