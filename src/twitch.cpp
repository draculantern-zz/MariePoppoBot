#ifndef TWICH_CPP
#define TWICH_CPP

#include "twitch.h"

FUNCTION void
twitch_wait_to_not_get_banned(f64 wallClockInSeconds)
{
    LOCAL_STATIC f64 lastTime = 0;
    
    f64 diffMs = (1000.0 * wallClockInSeconds) - (1000.0 * lastTime);
    
    // 300 bc mods can send 100 msgs in 30s to twitch w/o being global'd
    if (diffMs < 300.0)
    {
        PLATFORM_SLEEP((s32)(300.0 - diffMs) + 1);
    }
    lastTime = wallClockInSeconds;
}

FUNCTION void
twitch_format_pass_message(const char* pass, String* inout)
{
    string_clear(inout);
    append(inout, "PASS ");
    append(inout, pass);
    append(inout, "\r\n");
}

FUNCTION void
twitch_format_nick_message(const char* nick, String* inout)
{
    string_clear(inout);
    append(inout, "NICK ");
    append(inout, nick);
    append(inout, "\r\n");
}

FUNCTION void
twitch_format_join_message(const char* channel, String* inout)
{
    string_clear(inout);
    append(inout, "JOIN #");
    append(inout, channel);
    append(inout, "\r\n");
}

FUNCTION void
twitch_format_part_message(const char* channel, String* inout)
{
    string_clear(inout);
    append(inout, "PART #");
    append(inout, channel);
    append(inout, "\r\n");
}

FUNCTION void
twitch_format_channel_message(const TwitchMessage* twitchMessage,
                              String* inout)
{
    append(inout, "PRIVMSG #");
    append(inout, twitchMessage->channel);
    append(inout, " :");
    append(inout, twitchMessage->message);
    append(inout, "\r\n");
}

FUNCTION void
twitch_dispatch_callbacks(const TwitchClient* client, 
                          const char* twitchText, 
                          u32 twitchTextLength)
{
    char* parsedMsg = (char*)PLATFORM_MALLOC(twitchTextLength + 1);
    defer { PLATFORM_FREE(parsedMsg); };
    
    // please i hope twitch never gives us this many lol, otherwise we'll drop messages
    LOCAL_STATIC const s32 maxReceivedMsgs = 1024;
    LOCAL_STATIC char* receivedMsgs[maxReceivedMsgs];
    s32 receivedMsgsCount = 0;
    
    {
        memcpy(parsedMsg, twitchText, twitchTextLength);
        StringReader msgReader = make_string_reader(twitchText, twitchTextLength);
        auto crntTwitchMsgStart = 0;
        
        do
        {
            auto nextTwitchMsgStart = seek_next_line(&msgReader, NEWLINE_CR_LF);
            parsedMsg[nextTwitchMsgStart - 2] = 0;
            receivedMsgs[receivedMsgsCount] = parsedMsg + crntTwitchMsgStart;
            ++receivedMsgsCount;
            crntTwitchMsgStart = msgReader.cursor;
        } while(receivedMsgsCount < maxReceivedMsgs && valid_index(&msgReader, msgReader.cursor));
    }
    
    for(auto i = 0; i < receivedMsgsCount; ++i)
    {
        bool32 msgHandled = BOOL_FALSE;
        char* msg = receivedMsgs[i];
        StringReader msgReader = make_string_reader(msg);
        
        if (match("PING :tmi.twitch.tv", msg))
        {
            msgHandled = BOOL_TRUE;
            const char* pong = "PONG :tmi.twitch.tv\r\n";
            TwitchErrorCode pongResult = client->SendText(pong, (s32)strlen(pong));
            if (IRC_SOCKET_ERROR == pongResult)
            {
                PLATFORM_LOG("Received PING, but failed to PONG\n");
            }
        }
        else if (contains(msg, "PRIVMSG"))
        {
            msgHandled = BOOL_TRUE;
            
            // Read twitch tags
            TwitchUser user = {};
            
            {
                msgReader.cursor = 0;
                seek_next_string(&msgReader, "broadcaster/");
                if (next_char(&msgReader) == '1')
                {
                    user.channelBroadcaster = BOOL_TRUE;
                    user.channelModerator = BOOL_TRUE;
                }
                
                msgReader.cursor = 0;
                seek_next_string(&msgReader, "mod=");
                if (next_char(&msgReader) == '1')
                {
                    user.channelModerator = BOOL_TRUE;
                }
                
                msgReader.cursor = 0;
                seek_next_string(&msgReader, "subscriber=");
                if (next_char(&msgReader) == '1')
                {
                    user.channelSubscriber = BOOL_TRUE;
                }
                
                msgReader.cursor = 0;
                seek_next_string(&msgReader, "user-id=");
                s32 userIdStart = msgReader.cursor;
                user.id = parse_u64(msg + userIdStart);
                
                msgReader.cursor = 0;
                seek_next_string(&msgReader, "display-name=");
                s32 displayNameStart = seek_next_alphanumeric(&msgReader);
                s32 displayNameEnd   = seek_next_char(&msgReader, ';');
                memcpy(user.displayName,
                       msg + displayNameStart, 
                       min(displayNameEnd - displayNameStart, TWITCH_USER_NAME_LENGTH));
            }
            
            seek_next_string(&msgReader, "PRIVMSG");
            
            seek_next_char(&msgReader, '#');
            s32 channelStart = seek_next_alphanumeric(&msgReader);
            s32 channelEnd = seek_next_non_alphanumeric(&msgReader);
            
            seek_next_char(&msgReader, ':');
            s32 chatStart = seek_past_whitespace(&msgReader);
            s32 chatEnd = seek_next_char(&msgReader, 0);
            
            if (msg[chatStart] == client->cmdDesignator &&
                !char_is_whitespace(msg[chatStart + 1]))
            {
                msgReader.cursor = chatStart + 1;
                s32 cmdStart = chatStart + 1;
                s32 cmdEnd = seek_next_whitespace(&msgReader);
                chatStart = min(chatEnd, cmdEnd + 1);
                
                TwitchCommand twitchCommand = {};
                twitchCommand.user = user;
                twitchCommand.messageLength = chatEnd - chatStart; // + 1? 
                
                memcpy(twitchCommand.channel, 
                       msg + channelStart,
                       min(channelEnd - channelStart, TWITCH_USER_NAME_BUFFER - 1));
                memcpy(twitchCommand.cmd, 
                       msg + cmdStart,
                       min(cmdEnd - cmdStart, TWITCH_CMD_BUFFER - 1));
                memcpy(twitchCommand.cmdMessage, 
                       msg + chatStart,
                       min(chatEnd - chatStart, TWITCH_MESSAGE_BUFFER - 1));
                
                client->ReceiveTwitchCommand(&twitchCommand);
            }
            else
            {
                TwitchMessage twitchMessage = {};
                twitchMessage.user = user;
                twitchMessage.messageLength = chatEnd - chatStart;
                
                memcpy(twitchMessage.channel, 
                       msg + channelStart,
                       min(channelEnd - channelStart, TWITCH_USER_NAME_BUFFER - 1));
                memcpy(twitchMessage.message, 
                       msg + chatStart,
                       min(chatEnd - chatStart, TWITCH_MESSAGE_BUFFER - 1));
                
                client->ReceiveTwitchMessage(&twitchMessage);
            }
        }
        
        if (!msgHandled)
        {
            STACK_STRING(print, kilobytes(1));
            print << msg << "\n";
            PLATFORM_LOG(print.str);
        }
    }
}

#endif /* TWICH_CPP */