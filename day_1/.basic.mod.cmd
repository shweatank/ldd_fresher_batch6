savedcmd_basic.mod := printf '%s\n'   basic.o | awk '!x[$$0]++ { print("./"$$0) }' > basic.mod
