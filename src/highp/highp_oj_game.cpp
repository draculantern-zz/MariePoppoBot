#ifndef HIGHP_OJ_GAME_CPP
#define HIGHP_OJ_GAME_CPP

#define OJ_QUEUE_MAX_LENGTH 256

enum OjGameState
{
    OJ_GAME_NOT_STARTED,
    OJ_GAME_IN_PROGRESS,
    OJ_GAME_INTERMISSION,
};

struct OjTeamMember
{
    TwitchUser user;
    s32 team;
};

struct TeamLeader
{
    char name[32];
};

struct OjGame
{
    MemoryArena* memory;
    OjGameState state;
    TeamLeader teamLeaders[4];
    Array<OjTeamMember> registeredTeamMembers;
    Array<TwitchUser> queue;
};

GLOBAL PlatformApi OjPlatform;
GLOBAL OjGame Oj;


FUNCTION bool32
oj_team_members_match(const OjTeamMember& rhs, const OjTeamMember& lhs)
{
    return rhs.user.id == lhs.user.id;
}

FUNCTION bool32
twitch_users_match(const TwitchUser& rhs, const TwitchUser& lhs)
{
    return rhs.id == lhs.id;
}


FUNCTION HIGHP_COMMAND_FUNCTION(highp_commands)
{
    if (!twitchCommand->user.channelModerator ||
        !twitchCommand->user.channelBroadcaster)
    {
        return;
    }
    
    TwitchMessage* response = push<TwitchMessage>(Oj.memory);
    response->user = twitchCommand->user;
    memcpy(response->channel, twitchCommand->channel, TWITCH_USER_NAME_BUFFER);
    String commandsresponse = string_from_buffer(response->message, TWITCH_MESSAGE_BUFFER);
    
    commandsresponse <<
        "/me Commands list: !open !play !d4 !teams !nextround !winner "
        "!queue !remove !github !commands";
    
    response->messageLength = commandsresponse.length;
    client->SendTwitchMessage(response);
}

FUNCTION HIGHP_COMMAND_FUNCTION(highp_github)
{
    TwitchMessage* response = push<TwitchMessage>(Oj.memory);
    response->user = twitchCommand->user;
    memcpy(response->channel, twitchCommand->channel, TWITCH_USER_NAME_BUFFER);
    String githubresponse = string_from_buffer(response->message, TWITCH_MESSAGE_BUFFER);
    
    githubresponse << "/me If there is an issue with me, you can tell @draculantern; "
        "If you have a feature request or a bug report, you can let everyone "
        "know at https://github.com/draculantern/MariePoppoBot ulhuLove";
    
    response->messageLength = githubresponse.length;
    client->SendTwitchMessage(response);
}

FUNCTION HIGHP_COMMAND_FUNCTION(highp_open)
{
    TwitchMessage* response = push<TwitchMessage>(Oj.memory);
    response->user = twitchCommand->user;
    memcpy(response->channel, twitchCommand->channel, TWITCH_USER_NAME_BUFFER);
    String openresponse = string_from_buffer(response->message, TWITCH_MESSAGE_BUFFER);
    
    if (twitchCommand->user.channelBroadcaster || 
        twitchCommand->user.channelModerator)
    {
        Oj.state = OJ_GAME_INTERMISSION;
        
        openresponse << "/me The queue is open, type !play to enter";
        
        response->messageLength = openresponse.length;
        client->SendTwitchMessage(response);
    }
}

FUNCTION HIGHP_COMMAND_FUNCTION(highp_play)
{
    TwitchMessage* response = push<TwitchMessage>(Oj.memory);
    response->user = twitchCommand->user;
    memcpy(response->channel, twitchCommand->channel, TWITCH_USER_NAME_BUFFER);
    String playresponse = string_from_buffer(response->message, TWITCH_MESSAGE_BUFFER);
    
    TwitchUser* user = (TwitchUser*)&twitchCommand->user;
    
    if (OJ_GAME_NOT_STARTED == Oj.state)
    {
        playresponse
            << "/me @" << user->displayName 
            << " the queue is not open!";
    }
    else if (!twitchCommand->user.channelSubscriber &&
             !twitchCommand->user.channelBroadcaster)
    {
        playresponse
            << "/me @" << user->displayName 
            << " if you would like to play, you can subscribe for $4.99 at "
            "https://www.twitch.tv/products/queenulhu ulhuLove";
    }
    else // is a subscriber
    {
        s32 enqueuedIndex = array_find(&Oj.queue, *user, twitch_users_match);
        
        if (-1 == enqueuedIndex)
        {
            enqueuedIndex = array_add(&Oj.queue, *user);
        }
        
        playresponse
            << "/me @" << user->displayName 
            << " you are at position " << (enqueuedIndex+1) 
            << " in the queue.";
    }
    
    response->messageLength = playresponse.length;
    client->SendTwitchMessage(response);
}

FUNCTION HIGHP_COMMAND_FUNCTION(highp_d4)
{
    TwitchMessage* response = push<TwitchMessage>(Oj.memory);
    response->user = twitchCommand->user;
    memcpy(response->channel, twitchCommand->channel, TWITCH_USER_NAME_BUFFER);
    String d4response = string_from_buffer(response->message, TWITCH_MESSAGE_BUFFER);
    
    const TwitchUser* user = &twitchCommand->user;
    
    if (OJ_GAME_IN_PROGRESS != Oj.state)
    {
        d4response
            << "/me @" << user->displayName 
            << " I can't put you on a team because a game is not in progress ulhuW";
    }
    else 
    {
        for(TeamLeader* leader = Oj.teamLeaders;
            leader != &Oj.teamLeaders[ARRAY_LENGTH(Oj.teamLeaders)];
            ++leader)
        {
            if (!leader->name[0]) continue;
            if (match_ignore_case(leader->name, user->displayName))
            {
                d4response 
                    << "/me @" << user->displayName
                    << " you are the team leader, you can't roll you baka";
                goto HighpD4Response;
            }
        }
        
        OjTeamMember* teamMember = null;
        for_array(Oj.registeredTeamMembers)
        {
            if (it->user.id == user->id)
            {
                teamMember = &*it;
                break;
            }
        }
        
        s32 leaderNumber = 0;
        
        if (teamMember)
        {
            leaderNumber = teamMember->team;
            d4response
                << "/me @" << twitchCommand->user.displayName 
                << " you are already on team " 
                << leaderNumber
                << " for this game! ";
        }
        else
        {
            leaderNumber = rand_i32(1, 4);
            OjTeamMember newMember;
            newMember.team = leaderNumber;
            newMember.user = *user;
            s32 memberIndex = array_add(&Oj.registeredTeamMembers, newMember);
            teamMember = &Oj.registeredTeamMembers[memberIndex];
            
            d4response
                << "/me @" << twitchCommand->user.displayName  
                << " you are on team " << leaderNumber << "! ";
        }
        
        if (Oj.teamLeaders[leaderNumber - 1].name[0])
        {
            d4response << " Your leader is " << Oj.teamLeaders[leaderNumber - 1].name;
        }
    }
    
    HighpD4Response:
    
    response->messageLength = d4response.length;
    client->SendTwitchMessage(response);
}


FUNCTION HIGHP_COMMAND_FUNCTION(highp_teams)
{
    if (Oj.state != OJ_GAME_IN_PROGRESS) return;
    
    TwitchMessage* response = push<TwitchMessage>(Oj.memory);
    response->user = twitchCommand->user;
    memcpy(response->channel, twitchCommand->channel, TWITCH_USER_NAME_BUFFER);
    String teamsresponse = string_from_buffer(response->message, TWITCH_MESSAGE_BUFFER);
    
    // just an if that we can break out of
    while (twitchCommand->user.channelBroadcaster || 
           twitchCommand->user.channelModerator)
    {
        // try to parse the users
        StringReader leaderReader = make_string_reader((char*)twitchCommand->cmdMessage,
                                                       twitchCommand->messageLength);
        
        s32 oneStart = seek_next_alphanumeric(&leaderReader);
        s32 oneEnd   = seek_next_non_alphanumeric(&leaderReader);
        s32 twoStart = seek_next_alphanumeric(&leaderReader);
        s32 twoEnd   = seek_next_non_alphanumeric(&leaderReader);
        s32 threeStart = seek_next_alphanumeric(&leaderReader);
        s32 threeEnd   = seek_next_non_alphanumeric(&leaderReader);
        s32 fourStart = seek_next_alphanumeric(&leaderReader);
        s32 fourEnd   = seek_next_non_alphanumeric(&leaderReader);
        
        if ((oneEnd   - oneStart) <= 0 || 
            (twoEnd   - twoStart) <= 0 ||
            (threeEnd - threeStart) <= 0 ||
            (fourEnd  - fourStart) <= 0)
        {
            break;
        }
        
        memzero(Oj.teamLeaders, sizeof(Oj.teamLeaders));
        
        memcpy(Oj.teamLeaders[0].name,
               twitchCommand->cmdMessage + oneStart,
               oneEnd - oneStart);
        memcpy(Oj.teamLeaders[1].name,
               twitchCommand->cmdMessage + twoStart,
               twoEnd - twoStart);
        memcpy(Oj.teamLeaders[2].name,
               twitchCommand->cmdMessage + threeStart,
               threeEnd - threeStart);
        memcpy(Oj.teamLeaders[3].name,
               twitchCommand->cmdMessage + fourStart,
               fourEnd - fourStart);
        
        break;
    }
    
    if (!(Oj.teamLeaders[0].name[0])) 
    {
        teamsresponse << "/me The teams have not been set up!";
        goto HighpTeamsMessage;
    }
    
    teamsresponse << "/me The teams are"  
        << " 1. @" << Oj.teamLeaders[0].name 
        << " 2. @" << Oj.teamLeaders[1].name 
        << " 3. @" << Oj.teamLeaders[2].name 
        << " 4. @" << Oj.teamLeaders[3].name;
    
    HighpTeamsMessage:
    
    response->messageLength = teamsresponse.length;
    client->SendTwitchMessage(response);
}

FUNCTION HIGHP_COMMAND_FUNCTION(highp_winner)
{
    if (Oj.state != OJ_GAME_IN_PROGRESS) return;
    
    TwitchMessage* response = push<TwitchMessage>(Oj.memory);
    response->user = twitchCommand->user;
    memcpy(response->channel, twitchCommand->channel, TWITCH_USER_NAME_BUFFER);
    String winnertext = string_from_buffer(response->message, TWITCH_MESSAGE_BUFFER);
    
    u64 teamWinner = parse_u64(twitchCommand->cmdMessage);
    u64 winnerIndex = teamWinner - 1;
    if ((teamWinner < 1) || (teamWinner > 4))
    {
        winnertext << "/me You need to give me a proper team # you baka.";
        response->messageLength = winnertext.length;
        client->SendTwitchMessage(response);
        return;
    }
    
    if (!string_is_null_or_empty(Oj.teamLeaders[winnerIndex].name))
    {
        winnertext << "/me !payout " << Oj.teamLeaders[winnerIndex].name << " 1000";
        response->messageLength = winnertext.length;
        client->SendTwitchMessage(response);
    }
    
    memzero(Oj.teamLeaders, sizeof(Oj.teamLeaders));
    
    s32 winnerCount = 0;
    for_array(Oj.registeredTeamMembers)
    {
        if (it->team == teamWinner) winnerCount++;
    }
    winnerCount = max(1, winnerCount);
    
    for_array(Oj.registeredTeamMembers)
    {
        if (it->team == teamWinner)
        {
            string_clear(&winnertext);
            winnertext << "/me !payout " 
                << it->user.displayName << " " << (1000 / winnerCount);
            response->messageLength = winnertext.length;
            client->SendTwitchMessage(response);
        }
        it->team = 0;
    }
    
    string_clear(&winnertext);
    
    if (Oj.queue.count <= 0)
    {
        winnertext << "/me No one is left in the queue."
            " Wouldn't some little sub like to !play with me ulhuPraise";
    }
    else
    {
        winnertext << "/me @" << Oj.queue[0].displayName 
            << " you're up next in the queue, go to"
            " #hot-grills-club in the discord to get the password";
        
    }
    array_reset(&Oj.registeredTeamMembers);
    Oj.state = OJ_GAME_INTERMISSION;
    
    response->messageLength = winnertext.length;
    client->SendTwitchMessage(response);
}

FUNCTION HIGHP_COMMAND_FUNCTION(highp_nextround)
{
    if (OJ_GAME_IN_PROGRESS == Oj.state) return;
    
    Oj.state = OJ_GAME_IN_PROGRESS;
    if (Oj.queue.count > 0) array_remove_index_ordered(&Oj.queue, 0);
    
    TwitchMessage* response = push<TwitchMessage>(Oj.memory);
    response->user = twitchCommand->user;
    memcpy(response->channel, twitchCommand->channel, TWITCH_USER_NAME_BUFFER);
    String newroundtext = string_from_buffer(response->message, TWITCH_MESSAGE_BUFFER);
    
    newroundtext << "/me The game has started ulhuHype"
        " Type !d4 to see what team you're on."
        " Could a mod please set the !teams";
    
    response->messageLength = newroundtext.length;
    client->SendTwitchMessage(response);
}

FUNCTION HIGHP_COMMAND_FUNCTION(highp_queue)
{
    if (OJ_GAME_NOT_STARTED == Oj.state) return;
    
    TwitchMessage* response = push<TwitchMessage>(Oj.memory);
    response->user = twitchCommand->user;
    memcpy(response->channel, twitchCommand->channel, TWITCH_USER_NAME_BUFFER);
    String queuetext = string_from_buffer(response->message, TWITCH_MESSAGE_BUFFER);
    
    if (Oj.queue.count <= 0)
    {
        queuetext << "/me The queue is empty!";
    }
    else
    {
        for_array(Oj.queue)
        {
            if (queuetext.length + 32 >= TWITCH_MESSAGE_MAX_LENGTH) 
                break;
            queuetext << (it.index + 1) << ". " << it->displayName << " ";
        }
    }
    
    response->messageLength = queuetext.length;
    client->SendTwitchMessage(response);
}

FUNCTION HIGHP_COMMAND_FUNCTION(highp_remove)
{
    if (OJ_GAME_NOT_STARTED == Oj.state) return;
    if (!twitchCommand->cmdMessage[0]) return;
    
    
    TwitchMessage* response = push<TwitchMessage>(Oj.memory);
    response->user = twitchCommand->user;
    memcpy(response->channel, twitchCommand->channel, TWITCH_USER_NAME_BUFFER);
    String removetext = string_from_buffer(response->message, TWITCH_MESSAGE_BUFFER);
    
    bool32 foundUser = BOOL_FALSE;
    for_array(Oj.queue)
    {
        if (contains_ignore_case(twitchCommand->cmdMessage, it->displayName))
        {
            array_remove_index_ordered(&Oj.queue, it.index);
            foundUser = BOOL_TRUE;
            break;
        }
    }
    
    if (foundUser)
    {
        removetext << "/me " << twitchCommand->cmdMessage
            << " has been removed from the queue ulhuSlap";
    }
    else
    {
        removetext << "/me That person is not in the queue! ulhuS";
    }
    
    response->messageLength = removetext.length;
    client->SendTwitchMessage(response);
}

#endif /* HIGHP_OJ_GAME_CPP  */