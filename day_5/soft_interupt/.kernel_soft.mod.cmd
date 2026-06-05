savedcmd_kernel_soft.mod := printf '%s\n'   kernel_soft.o | awk '!x[$$0]++ { print("./"$$0) }' > kernel_soft.mod
