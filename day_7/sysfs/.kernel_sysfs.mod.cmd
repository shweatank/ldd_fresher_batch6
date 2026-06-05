savedcmd_kernel_sysfs.mod := printf '%s\n'   kernel_sysfs.o | awk '!x[$$0]++ { print("./"$$0) }' > kernel_sysfs.mod
