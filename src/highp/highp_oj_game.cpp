#ifndef HIGHP_OJ_GAME_CPP
#define HIGHP_OJ_GAME_CPP

#define OJ_QUEUE_MAX_LENGTH 256

enum OjGameStateOptions
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

//
// @TODO bring this into its own struct, for better access/serialization
//       when we need to write the contents to a file to be loaded later upon crash
GLOBAL OjGameStateOptions OjGameState;
GLOBAL String TeamLeaders[4];
GLOBAL Array<OjTeamMember> RegisteredTeamMembers;
GLOBAL Array<TwitchUser> OjQueue;


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

HIGHP_COMMAND_FUNCTION(highp_open)
{
    TwitchMessage response = {};
    response.user = twitchCommand->user;
    memcpy(response.channel, twitchCommand->channel, TWITCH_USER_NAME_BUFFER);
    String openresponse = string_from_buffer(TWITCH_MESSAGE_BUFFER, response.message);
    
    if (twitchCommand->user.channelBroadcaster || 
        twitchCommand->user.channelModerator)
    {
        OjGameState = OJ_GAME_INTERMISSION;
        
        openresponse << "/me The queue is open, type !play to enter";
        
        response.messageLength = openresponse.length;
        client->SendTwitchMessage(&response);
    }
}

HIGHP_COMMAND_FUNCTION(highp_play)
{
    TwitchMessage response = {};
    response.user = twitchCommand->user;
    memcpy(response.channel, twitchCommand->channel, TWITCH_USER_NAME_BUFFER);
    String playresponse = string_from_buffer(TWITCH_MESSAGE_BUFFER, response.message);
    
    TwitchUser* user = (TwitchUser*)&twitchCommand->user;
    
    if (OJ_GAME_NOT_STARTED == OjGameState)
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
        s32 enqueuedIndex = array_find(&OjQueue, *user, twitch_users_match);
        
        if (-1 == enqueuedIndex)
        {
            enqueuedIndex = array_add(&OjQueue, *user);
        }
        
        playresponse
            << "/me @" << user->displayName 
            << " you are at position " << (enqueuedIndex+1) 
            << " in the queue.";
    }
    
    response.messageLength = playresponse.length;
    client->SendTwitchMessage(&response);
}

HIGHP_COMMAND_FUNCTION(highp_d4)
{
    TwitchMessage response;
    response.user = twitchCommand->user;
    memcpy(response.channel, twitchCommand->channel, TWITCH_USER_NAME_BUFFER);
    String d4response = string_from_buffer(TWITCH_MESSAGE_BUFFER, response.message);
    
    const TwitchUser* user = &twitchCommand->user;
    
    if (OJ_GAME_IN_PROGRESS != OjGameState)
    {
        d4response
            << "/me @" << user->displayName 
            << " I can't put you on a team because a game is not in progress ulhuW";
    }
    else 
    {
        for(String* s = TeamLeaders;
            s != &TeamLeaders[ARRAY_LENGTH(TeamLeaders)];
            ++s)
        {
            if (s->length <= 0) continue;
            if (match_ignore_case(s, user->displayName))
            {
                d4response 
                    << "/me @" << user->displayName
                    << " you are the team leader, you can't roll you baka";
                goto HighpD4Response;
            }
        }
        
        OjTeamMember* teamMember = null;
        for_array(RegisteredTeamMembers)
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
            s32 memberIndex = array_add(&RegisteredTeamMembers, newMember);
            teamMember = &RegisteredTeamMembers[memberIndex];
            
            d4response
                << "/me @" << twitchCommand->user.displayName  
                << " you are on team " << leaderNumber << "! ";
        }
        
        if (TeamLeaders[leaderNumber - 1].str && TeamLeaders[leaderNumber - 1].str[0])
        {
            d4response << " Your leader is " << TeamLeaders[leaderNumber - 1].str;
        }
    }
    
    HighpD4Response:
    
    response.messageLength = d4response.length;
    client->SendTwitchMessage(&response);
}


HIGHP_COMMAND_FUNCTION(highp_teams)
{
    if (OjGameState != OJ_GAME_IN_PROGRESS) return;
    
    TwitchMessage response;
    response.user = twitchCommand->user;
    memcpy(response.channel, twitchCommand->channel, TWITCH_USER_NAME_BUFFER);
    String teamsresponse = string_from_buffer(TWITCH_MESSAGE_BUFFER, response.message);
    
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
        
        append_nchars(&TeamLeaders[0],
                      twitchCommand->cmdMessage + oneStart,
                      oneEnd - oneStart);
        append_nchars(&TeamLeaders[1],
                      twitchCommand->cmdMessage + twoStart,
                      twoEnd - twoStart);
        append_nchars(&TeamLeaders[2],
                      twitchCommand->cmdMessage + threeStart,
                      threeEnd - threeStart);
        append_nchars(&TeamLeaders[3],
                      twitchCommand->cmdMessage + fourStart,
                      fourEnd - fourStart);
        break;
    }
    
    if (!TeamLeaders[0].str || !(TeamLeaders[0].str[0])) 
    {
        teamsresponse << "/me The teams have not been set up!";
        goto HighpTeamsMessage;
    }
    
    teamsresponse << "/me The teams are"  
        << " 1. @" << TeamLeaders[0] 
        << " 2. @" << TeamLeaders[1] 
        << " 3. @" << TeamLeaders[2] 
        << " 4. @" << TeamLeaders[3];
    
    HighpTeamsMessage:
    
    response.messageLength = teamsresponse.length;
    client->SendTwitchMessage(&response);
}

HIGHP_COMMAND_FUNCTION(highp_winner)
{
    if (OjGameState != OJ_GAME_IN_PROGRESS) return;
    
    TwitchMessage response;
    response.user = twitchCommand->user;
    memcpy(response.channel, twitchCommand->channel, TWITCH_USER_NAME_BUFFER);
    String winnertext = string_from_buffer(TWITCH_MESSAGE_BUFFER, response.message);
    
    u64 teamWinner = parse_u64(twitchCommand->cmdMessage);
    u64 winnerIndex = teamWinner - 1;
    if ((teamWinner < 1) || (teamWinner > 4))
    {
        winnertext << "/me You need to give me a proper team # you baka.";
        response.messageLength = winnertext.length;
        client->SendTwitchMessage(&response);
        return;
    }
    
    if (!string_is_null_or_empty(&TeamLeaders[winnerIndex]))
    {
        winnertext << "/me !payout " << TeamLeaders[winnerIndex] << " 1000";
        response.messageLength = winnertext.length;
        client->SendTwitchMessage(&response);
    }
    
    string_clear(&TeamLeaders[0]);
    string_clear(&TeamLeaders[1]);
    string_clear(&TeamLeaders[2]);
    string_clear(&TeamLeaders[3]);
    
    s32 winnerCount = 0;
    for_array(RegisteredTeamMembers)
    {
        if (it->team == teamWinner) winnerCount++;
    }
    winnerCount = max(1, winnerCount);
    
    for_array(RegisteredTeamMembers)
    {
        if (it->team == teamWinner)
        {
            string_clear(&winnertext);
            winnertext << "/me !payout " 
                << it->user.displayName << " " << (1000 / winnerCount);
            response.messageLength = winnertext.length;
            client->SendTwitchMessage(&response);
        }
        it->team = 0;
    }
    
    string_clear(&winnertext);
    
    if (OjQueue.count <= 0)
    {
        winnertext << "/me No one is left in the queue."
            " Wouldn't some little sub like to !play with me ulhuPraise";
    }
    else
    {
        winnertext << "/me @" << OjQueue[0].displayName 
            << " you're up next in the queue, go to"
            " #hot-grills-club in the discord to get the password";
        
    }
    array_reset(&RegisteredTeamMembers);
    OjGameState = OJ_GAME_INTERMISSION;
    
    response.messageLength = winnertext.length;
    client->SendTwitchMessage(&response);
}

HIGHP_COMMAND_FUNCTION(highp_nextround)
{
    if (OJ_GAME_IN_PROGRESS == OjGameState) return;
    
    OjGameState = OJ_GAME_IN_PROGRESS;
    if (OjQueue.count > 0) array_remove_index_ordered(&OjQueue, 0);
    
    TwitchMessage response;
    response.user = twitchCommand->user;
    memcpy(response.channel, twitchCommand->channel, TWITCH_USER_NAME_BUFFER);
    String newroundtext = string_from_buffer(TWITCH_MESSAGE_BUFFER, response.message);
    
    newroundtext << "/me The game has started ulhuHype"
        " Type !d4 to see what team you're on."
        " Could a mod please set the !teams";
    
    response.messageLength = newroundtext.length;
    client->SendTwitchMessage(&response);
}

HIGHP_COMMAND_FUNCTION(highp_queue)
{
    if (OJ_GAME_NOT_STARTED == OjGameState) return;
    
    TwitchMessage response;
    response.user = twitchCommand->user;
    memcpy(response.channel, twitchCommand->channel, TWITCH_USER_NAME_BUFFER);
    String queuetext = string_from_buffer(TWITCH_MESSAGE_BUFFER, response.message);
    
    if (OjQueue.count <= 0)
    {
        queuetext << "/me The queue is empty!";
    }
    else
    {
        for_array(OjQueue)
        {
            if (queuetext.length + 32 >= TWITCH_MESSAGE_MAX_LENGTH) 
                break;
            queuetext << (it.index + 1) << ". " << it->displayName << " ";
        }
    }
    
    response.messageLength = queuetext.length;
    client->SendTwitchMessage(&response);
}

#endif /* HIGHP_OJ_GAME_CPP  */