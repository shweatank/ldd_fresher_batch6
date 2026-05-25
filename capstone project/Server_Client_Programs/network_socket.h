#ifndef NETWORK_SOCKET_H
#define NETWORK_SOCKET_H

int start_server(int port);
int connect_peer(const char *ip, int port);

#endif
