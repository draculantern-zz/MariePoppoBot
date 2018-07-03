#ifndef WIN32_IRC_CLIENT_H
#define WIN32_IRC_CLIENT_H

//#define DRAC_RANDOM_MERSENNE_TWISTER 1
#define DRAC_RANDOM_PCG 1

#include "win32_platform.h"
#include "platform.h"

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

#include "twitch.h"

struct Win32TwitchClient
{
    const char* url; 
    const char* port;
    char* nick;
    char* pass;
    char* channel;
    int sendFlags;
    
    SOCKET ircSocket;
    TwitchClient twitchClient;
};

#endif /* WIN32_IRC_CLIENT_H */

