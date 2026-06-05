savedcmd_kernel_waitq.mod := printf '%s\n'   kernel_waitq.o | awk '!x[$$0]++ { print("./"$$0) }' > kernel_waitq.mod
