#ifndef HIGHP_CPP
#define HIGHP_CPP

#include "platform.h"
#include "twitch.h"
#include "drac_array.h"
#include "drac_string.h"
#include "drac_random.h"

#define HIGHP_COMMAND_FUNCTION(fnName) \
void fnName(const TwitchClient* client, const TwitchCommand* twitchCommand)
typedef HIGHP_COMMAND_FUNCTION( (*PFN(HighpCommandFunction)) );


#include "highp.h"
#include "highp_oj_game.cpp"


struct HighpCommandMap
{
    const char* CommandName;
    PFN(HighpCommandFunction) HighpFunction;
};

#define HIGHP_COMMAND_MAP(cmdName, fnName) {cmdName, &fnName}
HighpCommandMap HighpCommandList[]
{
    HIGHP_COMMAND_MAP("commands", highp_commands),
    HIGHP_COMMAND_MAP("github", highp_github),
    HIGHP_COMMAND_MAP("open", highp_open),
    HIGHP_COMMAND_MAP("play", highp_play),
    HIGHP_COMMAND_MAP("d4", highp_d4),
    HIGHP_COMMAND_MAP("teams", highp_teams),
    HIGHP_COMMAND_MAP("winner", highp_winner),
    HIGHP_COMMAND_MAP("nextround", highp_nextround),
    HIGHP_COMMAND_MAP("queue", highp_queue),
    HIGHP_COMMAND_MAP("remove", highp_remove)
};

FUNCTION RECEIVE_TWITCH_MESSAGE_CALLBACK(highp_receive_message)
{
    STACK_STRING(format, kilobytes(1));
    format 
        << twitchMessage->user.displayName << ": " 
        << twitchMessage->message << '\n';
    PLATFORM_LOG(format.str);
}

FUNCTION RECEIVE_TWITCH_COMMAND_CALLBACK(highp_receive_command)
{
    begin_temp_stack(Oj.memory);
    defer { end_temp_stack(Oj.memory); };
    
    for(HighpCommandMap* cmdMap = HighpCommandList;
        cmdMap != &HighpCommandList[ARRAY_LENGTH(HighpCommandList)];
        ++cmdMap)
    {
        if (match(cmdMap->CommandName, twitchCommand->cmd))
        {
            cmdMap->HighpFunction(client, twitchCommand);
            break;
        }
    }
    
    String format = push_string(Oj.memory, kilobytes(1));
    
    format 
        << twitchCommand->user.displayName << ": "
        << client->cmdDesignator << twitchCommand->cmd;
    
    if (twitchCommand->cmdMessage[0])
        format  << ' ' << twitchCommand->cmdMessage;
    format << '\n';
    
    PLATFORM_LOG(format.str);
}

extern "C" void
init_client(TwitchClient* out)
{
    out->cmdDesignator = '!';
    twitch_client_set_callback(*out, ReceiveTwitchMessage, &highp_receive_message);
    twitch_client_set_callback(*out, ReceiveTwitchCommand, &highp_receive_command);
    
    // Oj initialization
    {
        const u32 ojMaxQueueLength = 64;
        const u32 ojMaxRegisteredTeamMembers = 1024;
        
        Oj = {};
        Oj.state = OJ_GAME_NOT_STARTED;
        Oj.memory = allocate_memory_arena(megabytes(1) +
                                          (ojMaxQueueLength * sizeof(TwitchUser)) +
                                          (ojMaxRegisteredTeamMembers * sizeof(OjTeamMember)));
        
        Oj.registeredTeamMembers = 
            push_array<OjTeamMember>(Oj.memory, ojMaxRegisteredTeamMembers);
        
        Oj.queue = 
            push_array<TwitchUser>(Oj.memory, ojMaxQueueLength);
        
    }
}


#endif /* HIGHP_CPP */
