savedcmd_kernel.mod := printf '%s\n'   kernel.o | awk '!x[$$0]++ { print("./"$$0) }' > kernel.mod
