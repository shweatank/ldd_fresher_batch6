# Userspace Secure Access menu application
#
# Depends on OpenSSL libcrypto for EVP SHA-256 hashing.
# Build: make  -> produces secure_access_app
# Run:   sudo ./secure_access_app   (needs access to /dev/secure_access0 + DB path)

CC ?= gcc
CFLAGS += -Wall -Wextra -O2
LDFLAGS += -lcrypto

all: secure_access_app

secure_access_app: secure_access_app.c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -f secure_access_app
