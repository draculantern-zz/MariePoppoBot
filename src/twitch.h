#ifndef TWITCH_H
#define TWITCH_H

#include "platform.h"
#include "drac_memory.h"
#include "drac_string.h"

struct TwitchClient;
struct TwitchCommand;
struct TwitchMessage;
struct TwitchUser;


enum TwitchErrorCode
{
    TWITCH_SUCCESS = 0,
    IRC_SOCKET_ERROR,
};


#define RECEIVE_TWITCH_MESSAGE_CALLBACK(fnName) \
void fnName(const TwitchClient* client, const TwitchMessage* twitchMessage)
typedef RECEIVE_TWITCH_MESSAGE_CALLBACK( (*PFN(ReceiveTwitchMessage)) );

#define RECEIVE_TWITCH_COMMAND_CALLBACK(fnName) \
void fnName(const TwitchClient* client, const TwitchCommand* twitchCommand)
typedef RECEIVE_TWITCH_COMMAND_CALLBACK( (*PFN(ReceiveTwitchCommand)) );

typedef void TwitchPlatformData;
#define SEND_TWITCH_MESSAGE_CALLBACK(fnName) \
TwitchErrorCode fnName(const TwitchClient* client,const TwitchMessage* twitchMessage)
typedef SEND_TWITCH_MESSAGE_CALLBACK( (*PFN(SendTwitchMessage)) );

#define SEND_TWITCH_TEXT_CALLBACK(fnName) \
TwitchErrorCode fnName(const TwitchClient* client, \
const char* text,\
s32 textLength)
typedef SEND_TWITCH_TEXT_CALLBACK( (*PFN(SendText)) );

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


#define TWITCH_CALLBACK_NAME(fn) _##fn
#define DECLARE_TWITCH_CALLBACK(fn) PFN(fn) TWITCH_CALLBACK_NAME(fn)

#define TWITCH_CLIENT_DISPATCH(fn, ...) \
TWITCH_CALLBACK_NAME(fn) (this, __VA_ARGS__);

struct TwitchClient
{
    MemoryArena* arena;
    DECLARE_TWITCH_CALLBACK(ReceiveTwitchMessage);
    DECLARE_TWITCH_CALLBACK(ReceiveTwitchCommand);
    DECLARE_TWITCH_CALLBACK(SendTwitchMessage);
    DECLARE_TWITCH_CALLBACK(SendText);
    char cmdDesignator;
    TwitchPlatformData* reserved;
    
    FORCE_INLINE void 
        ReceiveTwitchMessage(const TwitchMessage* twitchMessage) const
    {
        TWITCH_CLIENT_DISPATCH(ReceiveTwitchMessage, twitchMessage);
    }
    FORCE_INLINE void 
        ReceiveTwitchCommand(const TwitchCommand* twitchCommand) const
    {
        TWITCH_CLIENT_DISPATCH(ReceiveTwitchCommand, twitchCommand);
    }
    FORCE_INLINE TwitchErrorCode 
        SendTwitchMessage(const TwitchMessage* twitchMessage) const
    {
        return TWITCH_CLIENT_DISPATCH(SendTwitchMessage, twitchMessage);
    }
    FORCE_INLINE TwitchErrorCode
        SendText(const char* text, s32 textLength) const
    {
        return TWITCH_CLIENT_DISPATCH(SendText, text, textLength);
    }
};

#define twitch_client_set_callback(client, cbk, pfn) \
(client). TWITCH_CALLBACK_NAME(cbk) = pfn

FUNCTION void twitch_wait_to_not_get_banned(f64 wallClockInSeconds);
FUNCTION void twitch_format_pass_message(const char* pass, String* inout);
FUNCTION void twitch_format_nick_message(const char* nick, String* inout);
FUNCTION void twitch_format_join_message(const char* channel, String* inout);
FUNCTION void twitch_format_part_message(const char* channel, String* inout);
FUNCTION void twitch_format_channel_message(const TwitchMessage* twitchMessage,
                                            String* inout);
FUNCTION void twitch_dispatch_callbacks(const TwitchClient* client, 
                                        const char* twitchText, 
                                        u32 twitchTextLength);

#endif /* TWITCH_H */
