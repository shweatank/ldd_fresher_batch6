savedcmd_ioctl_drv_running_sum.mod := printf '%s\n'   ioctl_drv_running_sum.o | awk '!x[$$0]++ { print("./"$$0) }' > ioctl_drv_running_sum.mod
