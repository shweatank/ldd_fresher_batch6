savedcmd_ioctl_drv_kmalloc.mod := printf '%s\n'   ioctl_drv_kmalloc.o | awk '!x[$$0]++ { print("./"$$0) }' > ioctl_drv_kmalloc.mod
