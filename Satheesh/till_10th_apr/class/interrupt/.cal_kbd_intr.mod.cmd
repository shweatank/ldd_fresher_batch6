savedcmd_cal_kbd_intr.mod := printf '%s\n'   cal_kbd_intr.o | awk '!x[$$0]++ { print("./"$$0) }' > cal_kbd_intr.mod
