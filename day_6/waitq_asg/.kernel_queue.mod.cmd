savedcmd_kernel_queue.mod := printf '%s\n'   kernel_queue.o | awk '!x[$$0]++ { print("./"$$0) }' > kernel_queue.mod
