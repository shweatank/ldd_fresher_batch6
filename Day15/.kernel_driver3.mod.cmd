savedcmd_kernel_driver3.mod := printf '%s\n'   kernel_driver3.o | awk '!x[$$0]++ { print("./"$$0) }' > kernel_driver3.mod
