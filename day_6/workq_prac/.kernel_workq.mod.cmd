savedcmd_kernel_workq.mod := printf '%s\n'   kernel_workq.o | awk '!x[$$0]++ { print("./"$$0) }' > kernel_workq.mod
