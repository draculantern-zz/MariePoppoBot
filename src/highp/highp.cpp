#ifndef HIGHP_CPP
#define HIGHP_CPP

#include "platform.h"
#include "twitch.h"
#include "drac_array.h"
#include "drac_string.h"
#include "drac_random.h"

#define HIGHP_COMMAND_FUNCTION(fnName)           \
FUNCTION void fnName(const TwitchClient* client, \
const TwitchCommand* twitchCommand)

typedef void (*PFN(HighpCommandFunction))(const TwitchClient* client,
                                          const TwitchCommand* twitchCommand);


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
    HIGHP_COMMAND_MAP("open", highp_open),
    HIGHP_COMMAND_MAP("play", highp_play),
    HIGHP_COMMAND_MAP("d4", highp_d4),
    HIGHP_COMMAND_MAP("teams", highp_teams),
    HIGHP_COMMAND_MAP("winner", highp_winner),
    HIGHP_COMMAND_MAP("nextround", highp_nextround),
    HIGHP_COMMAND_MAP("queue", highp_queue),
};

RECEIVE_TWITCH_MESSAGE_CALLBACK(highp_receive_message)
{
    STACK_STRING(format, kilobytes(1));
    format 
        << twitchMessage->user.displayName << ": " 
        << twitchMessage->message << '\n';
    PLATFORM_LOG(format.str);
}

RECEIVE_TWITCH_COMMAND_CALLBACK(highp_receive_command)
{
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
    
    STACK_STRING(format, kilobytes(1));
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
    
    {
        for(String* teamString = TeamLeaders;
            teamString != &TeamLeaders[ARRAY_LENGTH(TeamLeaders)];
            ++teamString)
        {
            *teamString = allocate_string();
        }
        
        RegisteredTeamMembers = allocate_array<OjTeamMember>();
        OjQueue = allocate_array<TwitchUser>();
        
        OjGameState = OJ_GAME_NOT_STARTED;
    }
}


#endif /* HIGHP_CPP */
