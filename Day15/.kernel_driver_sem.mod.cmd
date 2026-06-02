savedcmd_kernel_driver_sem.mod := printf '%s\n'   kernel_driver_sem.o | awk '!x[$$0]++ { print("./"$$0) }' > kernel_driver_sem.mod
