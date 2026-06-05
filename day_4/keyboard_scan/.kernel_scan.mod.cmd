savedcmd_kernel_scan.mod := printf '%s\n'   kernel_scan.o | awk '!x[$$0]++ { print("./"$$0) }' > kernel_scan.mod
