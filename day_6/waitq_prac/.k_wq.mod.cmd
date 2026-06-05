savedcmd_k_wq.mod := printf '%s\n'   k_wq.o | awk '!x[$$0]++ { print("./"$$0) }' > k_wq.mod
