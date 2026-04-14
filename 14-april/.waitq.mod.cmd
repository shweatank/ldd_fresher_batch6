savedcmd_waitq.mod := printf '%s\n'   waitq.o | awk '!x[$$0]++ { print("./"$$0) }' > waitq.mod
