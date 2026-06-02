savedcmd_kernel_driver2.mod := printf '%s\n'   kernel_driver2.o | awk '!x[$$0]++ { print("./"$$0) }' > kernel_driver2.mod
