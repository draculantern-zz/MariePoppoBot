#ifndef TWITCH_H
#define TWITCH_H

#include "platform.h"

struct TwitchClient;
struct TwitchCommand;
struct TwitchMessage;
struct TwitchUser;


enum TwitchErrorCode
{
    TWITCH_SUCCESS = 0,
    IRC_SOCKET_ERROR,
};


#define RECEIVE_TWITCH_MESSAGE_CALLBACK(fnName)  \
FUNCTION void fnName(const TwitchClient* client, \
const TwitchMessage* twitchMessage)
typedef void (*PFN(ReceiveTwitchMessage))(const TwitchClient* client,
                                          const TwitchMessage* twitchMessage);

#define RECEIVE_TWITCH_COMMAND_CALLBACK(fnName)  \
FUNCTION void fnName(const TwitchClient* client, \
const TwitchCommand* twitchCommand)
typedef void (*PFN(ReceiveTwitchCommand))(const TwitchClient* client,
                                          const TwitchCommand* twitchCommand);

typedef void TwitchPlatformData;
#define SEND_TWITCH_MESSAGE_CALLBACK(fnName)                \
FUNCTION TwitchErrorCode fnName(const TwitchClient* client, \
const TwitchMessage* twitchMessage)
typedef TwitchErrorCode (*PFN(SendTwitchMessage))(const TwitchClient* client,
                                                  const TwitchMessage* twitchMessage);

#define TWITCH_MESSAGE_MAX_LENGTH 500
#define TWITCH_MESSAGE_BUFFER 512
#define TWITCH_USER_NAME_LENGTH 20
#define TWITCH_USER_NAME_BUFFER 32
#define TWITCH_CMD_BUFFER 32

struct TwitchUser
{
    u64 id;
    char displayName[TWITCH_USER_NAME_BUFFER];
    bool32 channelBroadcaster;
    bool32 channelModerator;
    bool32 channelSubscriber;
};

struct TwitchMessage
{
    TwitchUser user;
    char channel[TWITCH_USER_NAME_BUFFER];
    char message[TWITCH_MESSAGE_BUFFER];
    s32 messageLength;
};

struct TwitchCommand
{
    TwitchUser user;
    char channel[TWITCH_MESSAGE_BUFFER];
    char cmd[TWITCH_CMD_BUFFER];
    char cmdMessage[TWITCH_MESSAGE_BUFFER];
    s32 messageLength;
};

#define DECLARE_TWITCH_CALLBACK(fn) PFN(fn) fn
struct TwitchClient
{
    DECLARE_TWITCH_CALLBACK(ReceiveTwitchMessage);
    DECLARE_TWITCH_CALLBACK(ReceiveTwitchCommand);
    const DECLARE_TWITCH_CALLBACK(SendTwitchMessage);
    char cmdDesignator;
    TwitchPlatformData* reserved;
};


#endif /* TWITCH_H */
