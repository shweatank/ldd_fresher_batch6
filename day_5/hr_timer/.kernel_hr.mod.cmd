savedcmd_kernel_hr.mod := printf '%s\n'   kernel_hr.o | awk '!x[$$0]++ { print("./"$$0) }' > kernel_hr.mod
