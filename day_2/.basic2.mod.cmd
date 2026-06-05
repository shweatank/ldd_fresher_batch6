savedcmd_basic2.mod := printf '%s\n'   basic2.o | awk '!x[$$0]++ { print("./"$$0) }' > basic2.mod
