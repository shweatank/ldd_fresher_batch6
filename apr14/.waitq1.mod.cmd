savedcmd_waitq1.mod := printf '%s\n'   waitq1.o | awk '!x[$$0]++ { print("./"$$0) }' > waitq1.mod
