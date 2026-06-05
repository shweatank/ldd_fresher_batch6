savedcmd_kernel_wq.mod := printf '%s\n'   kernel_wq.o | awk '!x[$$0]++ { print("./"$$0) }' > kernel_wq.mod
