savedcmd_kernel_mutex.mod := printf '%s\n'   kernel_mutex.o | awk '!x[$$0]++ { print("./"$$0) }' > kernel_mutex.mod
