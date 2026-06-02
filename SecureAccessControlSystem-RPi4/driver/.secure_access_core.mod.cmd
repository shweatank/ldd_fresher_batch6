savedcmd_secure_access_core.mod := printf '%s\n'   sac_core_main.o sac_tft.o sac_irq.o sac_uart.o | awk '!x[$$0]++ { print("./"$$0) }' > secure_access_core.mod
