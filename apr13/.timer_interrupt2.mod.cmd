savedcmd_timer_interrupt2.mod := printf '%s\n'   timer_interrupt2.o | awk '!x[$$0]++ { print("./"$$0) }' > timer_interrupt2.mod
