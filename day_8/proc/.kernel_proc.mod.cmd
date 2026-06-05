savedcmd_kernel_proc.mod := printf '%s\n'   kernel_proc.o | awk '!x[$$0]++ { print("./"$$0) }' > kernel_proc.mod
