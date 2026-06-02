savedcmd_kernel_driver.mod := printf '%s\n'   kernel_driver.o | awk '!x[$$0]++ { print("./"$$0) }' > kernel_driver.mod
