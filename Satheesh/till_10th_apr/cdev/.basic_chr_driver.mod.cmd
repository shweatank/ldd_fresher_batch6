savedcmd_basic_chr_driver.mod := printf '%s\n'   basic_chr_driver.o | awk '!x[$$0]++ { print("./"$$0) }' > basic_chr_driver.mod
