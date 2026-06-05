savedcmd_kernel_inter.mod := printf '%s\n'   kernel_inter.o | awk '!x[$$0]++ { print("./"$$0) }' > kernel_inter.mod
