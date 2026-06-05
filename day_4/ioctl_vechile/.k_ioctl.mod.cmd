savedcmd_k_ioctl.mod := printf '%s\n'   k_ioctl.o | awk '!x[$$0]++ { print("./"$$0) }' > k_ioctl.mod
