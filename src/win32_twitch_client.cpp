#include "win32_twitch_client.h"

#include "platform.h"
#include "drac_array.h"
#include "drac_math.h"
#include "drac_random.h"
#include "drac_string.h"

#include "highp/highp.cpp"
#include "drac_random.cpp"


#define RECEIVE_BUFFER_LENGTH (megabytes(1))


//
// @TODO using the heap is bad, lol
//       this is a tiny baby program, I'll start refactoring this out to 
//       real memory allocation when queen starts requesting more features
//
GLOBAL HANDLE ProcessHeap = null;

FUNCTION void* 
win32_heap_alloc(u64 numbytes) 
{
    if (!ProcessHeap) ProcessHeap = GetProcessHeap();
    numbytes = align8(numbytes);
    return HeapAlloc(ProcessHeap, HEAP_ZERO_MEMORY, numbytes);
}

FUNCTION void* 
win32_heap_realloc(void* data, u64 numbytes) 
{
    numbytes = align8(numbytes);
    
    void* firstTryRealloc =
        HeapReAlloc(ProcessHeap, 
                    HEAP_ZERO_MEMORY | HEAP_REALLOC_IN_PLACE_ONLY, 
                    data, 
                    numbytes);
    
    if (firstTryRealloc) return firstTryRealloc;
    
    return HeapReAlloc(ProcessHeap, 
                       HEAP_ZERO_MEMORY, 
                       data, 
                       numbytes);
}

FUNCTION void 
win32_heap_free(void* data)
{
    HeapFree(ProcessHeap, 0, data);
}

FUNCTION void 
win32_print(const char* buf)
{
    LOCAL_STATIC HANDLE StdOut = null;
    if (!StdOut) 
    {
        StdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    }
    WriteFile(StdOut, 
              buf, (DWORD)strlen(buf),
              null, null);
}

FUNCTION s32
win32_try_connect_twitch(Win32TwitchClient* client)
{
    STACK_STRING(err, 1024);
    
    ADDRINFOA addrHints = {};
    addrHints.ai_family = AF_UNSPEC;
    addrHints.ai_socktype = SOCK_STREAM;
    addrHints.ai_protocol = IPPROTO_TCP;
    
    ADDRINFOA* addrLL;
    
    if (0 != getaddrinfo(client->url, client->port, &addrHints, &addrLL))
    {
        string_clear(&err);
        err << "getaddrinfo failed for " << client->url << client->port << "\n";
        win32_print(err.str);
        goto ConnectError;
    }
    
    // test if we can connect to any of the potential address infos
    for ( ; addrLL != NULL; addrLL = addrLL->ai_next)
    {
        win32_print("...connecting to twitch...\n");
        
        // create a socket
        client->ircSocket = socket(addrLL->ai_family,
                                   addrLL->ai_socktype,
                                   addrLL->ai_protocol);
        
        if (INVALID_SOCKET == client->ircSocket)
        {
            string_clear(&err);
            err << "socket failed with error: " << WSAGetLastError();
            win32_print(err.str);
            goto ConnectError;
        }
        
        // try to connect with socket
        if (SOCKET_ERROR == connect(client->ircSocket, 
                                    addrLL->ai_addr, 
                                    (int)addrLL->ai_addrlen))
        {
            closesocket(client->ircSocket);
            client->ircSocket = INVALID_SOCKET;
        }
        else
        {
            break;
        }
    }
    
    freeaddrinfo(addrLL);
    
    if (INVALID_SOCKET == client->ircSocket)
    {
        string_clear(&err);
        err << "socket failed with error: " << WSAGetLastError();
        win32_print(err.str);
        goto ConnectError;
    }
    
    
    // login
    String sendString = allocate_string();
    sendString << "PASS " << client->pass << "\n";
    
    int passResult =send(client->ircSocket, sendString.str, (int)sendString.length, 0);
    
    string_clear(&sendString);
    sendString << "NICK " << client->nick << "\n";
    
    int nickResult = send(client->ircSocket, sendString.str, (int)sendString.length, 0);
    
    if ((SOCKET_ERROR == passResult) || (SOCKET_ERROR == nickResult))
    {
        string_clear(&err);
        err << "Unable to login to twitch on account ("
            << client->nick << ")\n failed with error code:" << WSAGetLastError();
        win32_print(err.str);
        goto ConnectError;
    }
    
    
    string_clear(&sendString);
    sendString << "JOIN #" << client->channel << "\n";
    
    int joinResult = send(client->ircSocket, sendString.str, (int)sendString.length, 0);
    
    if (SOCKET_ERROR == joinResult)
    {
        string_clear(&err);
        err << "Unable to connect to twitch channel ("
            << client->channel << ")\n failed with error code:" << WSAGetLastError();
        win32_print(err.str);
        goto ConnectError;
    }
    
    return 0;
    
    ConnectError:
    return -1;
}

SEND_TWITCH_MESSAGE_CALLBACK(win32_send_message)
{
    LOCAL_STATIC s64 lastPerfCounter = 0;
    LOCAL_STATIC s64 perfFrequency = 0;
    
    if (!perfFrequency)
    {
        LARGE_INTEGER freqLargeInt;
        QueryPerformanceFrequency(&freqLargeInt);
        perfFrequency = freqLargeInt.QuadPart;
    }
    
    Win32TwitchClient* win32Client = (Win32TwitchClient*)client->reserved;
    
    STACK_STRING(formattedMessage, 256 + twitchMessage->messageLength);
    
    formattedMessage << "PRIVMSG #" << twitchMessage->channel
        << " :" << twitchMessage->message << "\r\n";
    
    
    LARGE_INTEGER counterLargeInt;
    QueryPerformanceCounter(&counterLargeInt);
    s64 crntPerfCounter = counterLargeInt.QuadPart;
    
    s64 diffTicks = crntPerfCounter - lastPerfCounter;
    f64 diffMs = 1000.0 * (f64)diffTicks / perfFrequency;
    lastPerfCounter = crntPerfCounter;
    
    // 300 bc mods can send 100 msgs in 30s to twitch w/o being global'd
    if (diffMs < 300.0)
    {
        Sleep((s32)(300.0 - diffMs) + 1);
    }
    
    s32 result = send(win32Client->ircSocket, 
                      formattedMessage.str, 
                      (int)formattedMessage.length,
                      win32Client->sendFlags);
    
    if (SOCKET_ERROR == result)
    {
        return IRC_SOCKET_ERROR;
    }
    return TWITCH_SUCCESS;
}

FUNCTION void
win32_dispatch_twitch_callbacks(const Win32TwitchClient* win32Client, 
                                const char* twitchMsg, 
                                u32 twitchMsgLength)
{
    LOCAL_STATIC char parsedMsg[RECEIVE_BUFFER_LENGTH];
    
    // please i hope twitch never gives us this many lol, otherwise we'll drop messages
    LOCAL_STATIC const s32 maxReceivedMsgs = 1024;
    LOCAL_STATIC char* receivedMsgs[maxReceivedMsgs];
    s32 receivedMsgsCount = 0;
    
    {
        memcpy(parsedMsg, twitchMsg, twitchMsgLength);
        StringReader msgReader = make_string_reader(twitchMsg, twitchMsgLength);
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
            int pongResult = send(win32Client->ircSocket, 
                                  pong, (int)strlen(pong), 
                                  win32Client->sendFlags);
            if (SOCKET_ERROR == pongResult)
            {
                win32_print("Received PING, but failed to PONG\n");
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
            
            if (msg[chatStart] == win32Client->twitchClient.cmdDesignator &&
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
                       min(cmdEnd - channelStart, TWITCH_CMD_BUFFER - 1));
                memcpy(twitchCommand.cmdMessage, 
                       msg + chatStart,
                       min(chatEnd - chatStart, TWITCH_MESSAGE_BUFFER - 1));
                
                win32Client->twitchClient.ReceiveTwitchCommand(&win32Client->twitchClient,
                                                               &twitchCommand);
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
                
                win32Client->twitchClient.ReceiveTwitchMessage(&win32Client->twitchClient,
                                                               &twitchMessage);
            }
        }
        
        if (!msgHandled)
        {
            STACK_STRING(print, kilobytes(1));
            print << msg << "\n";
            win32_print(print.str);
        }
    }
    
    memzero(parsedMsg, twitchMsgLength);
}

extern "C" void 
mainCRTStartup()
{
    //
    // Random init
    //
    LARGE_INTEGER qpc;
    QueryPerformanceCounter(&qpc);
    
    // use address of platform function for entropy
    seed_rand(qpc.QuadPart);
    
    //
    // Platform Client
    //
    Win32TwitchClient Client = {};
    
    Client.ircSocket = INVALID_SOCKET;
    Client.url = "irc.chat.twitch.tv";
    Client.port = "6667";
    Client.sendFlags = 0;
    
    //
    // Config options
    //
    HANDLE configFile = CreateFile("bot.config", 
                                   GENERIC_READ,
                                   FILE_SHARE_READ,
                                   NULL,
                                   OPEN_EXISTING,
                                   0, NULL);
    
    if (!configFile)
    {
        win32_print("Could not find 'bot.config', what the heck\n");
        goto Close;
    }
    
    DWORD configFileSize = kilobytes(1);
    //GetFileSize(configFile, &configFileSize);
    
    char* configBuffer = (char*)alloca(configFileSize + 1);
    configBuffer[configFileSize] = 0;
    DWORD countBytesIn;
    ReadFile(configFile, configBuffer, configFileSize, &countBytesIn, NULL);
    CloseHandle(configFile);
    
    {
        StringReader configReader = make_string_reader((char*)configBuffer);
        
        do
        {
            if (configBuffer[configReader.cursor] == '#') continue;
            
            s32 startConfigOption = seek_next_alpha(&configReader);
            
            if (match("nick", (const char*)configBuffer + startConfigOption))
            {
                seek_next_char(&configReader, ':');
                s32 startConfigValue = seek_past_whitespace(&configReader);
                s32 endConfigValue = seek_next_whitespace(&configReader);
                configBuffer[endConfigValue] = 0;
                
                Client.nick = configBuffer + startConfigValue;
                to_lower(Client.nick);
            } 
            else if (match("pass", (const char*)configBuffer + startConfigOption)) 
            {
                seek_next_char(&configReader, ':');
                s32 startConfigValue = seek_past_whitespace(&configReader);
                s32 endConfigValue = seek_next_whitespace(&configReader);
                configBuffer[endConfigValue] = 0;
                
                Client.pass = configBuffer + startConfigValue;
            } 
            else if (match("channel", (const char*)configBuffer + startConfigOption)) 
            {
                seek_next_char(&configReader, ':');
                s32 startConfigValue = seek_past_whitespace(&configReader);
                s32 endConfigValue = seek_next_whitespace(&configReader);
                
                configBuffer[endConfigValue] = 0;
                
                Client.channel = configBuffer + startConfigValue;
                to_lower(Client.channel);
            }
            
        } while(seek_next_line(&configReader, LINE_FEED) < configReader.length);
    }
    
    STACK_STRING(err, kilobytes(64));
    
    //
    // Setup a console for the current Process 
    //
    HWND console = GetConsoleWindow();
    if (!console)
    {
        AllocConsole();
        console = GetConsoleWindow();
    }
    
    
    WSADATA wsaData;
    WORD wsaVersion = MAKEWORD(2, 2);
    if (0 != WSAStartup(wsaVersion, &wsaData))
    {
        wsaVersion = MAKEWORD(2, 0);
        if (0 != WSAStartup(wsaVersion, &wsaData))
        {
            win32_print("Failed to load WinSocket\n");
            goto Close;
        }
    }
    
    s32 successCode = win32_try_connect_twitch(&Client);
    
    if (0 != successCode)
    {
        string_clear(&err);
        err << "Failed to connect " << Client.nick << ". Terminating...\n";
        win32_print(err.str);
        goto Close;
    }
    
    Client.twitchClient.reserved = &Client;
    Client.twitchClient.cmdDesignator = '!';
    
    // yuck... but had to do it to allow SendTwitchMessage to be const 
    PFN(SendTwitchMessage)* sendCbck = 
        (PFN(SendTwitchMessage)*)&Client.twitchClient.SendTwitchMessage;
    *sendCbck = &win32_send_message;
    
    init_client(&Client.twitchClient);
    
    
    const char* getTagsMsg = "CAP REQ :twitch.tv/tags\r\n";
    int tagsResult = send(Client.ircSocket, 
                          getTagsMsg, (int)strlen(getTagsMsg), 
                          Client.sendFlags);
    
    if (SOCKET_ERROR == tagsResult)
    {
        string_clear(&err);
        err << "Failed to ask for user tags with messages from twitch\n";
        win32_print(err.str);
        goto Close;
    }
    
    char* receiveBuffer = (char*)VirtualAlloc(0, 
                                              RECEIVE_BUFFER_LENGTH, 
                                              MEM_COMMIT | MEM_RESERVE,
                                              PAGE_READWRITE);
    assert(receiveBuffer);
    
    const int recvFlags = 0;
    bool32 isConnected = BOOL_TRUE;
    
    // pump irc
    while (isConnected)
    {
        string_clear(&err);
        
        // @TODO use select() for timeout checks???
        //       this might be a bad idea bc we don't receive anything but PING
        //       for chats with no one active
        int nbytes = recv(Client.ircSocket, 
                          receiveBuffer,
                          RECEIVE_BUFFER_LENGTH - 1,
                          recvFlags);
        
        if (0 < nbytes)
        {
            receiveBuffer[nbytes] = 0;
            win32_dispatch_twitch_callbacks(&Client, receiveBuffer, nbytes);
        }
        else if (0 == recvFlags)
        {
            err << "connection closed: " << WSAGetLastError() << "\n";
            win32_print(err.str);
            isConnected = BOOL_FALSE;
        }
        else
        {
            err << "recv failed with error code: " << WSAGetLastError() << "\n";
            win32_print(err.str);
        }
        
        // reconnect loop
        if (!isConnected)
        {
            const s32 maxRetries = 4;
            s32 retryCount = 0;
            
            while (!isConnected || retryCount < maxRetries)
            {
                string_clear(&err);
                retryCount++;
                
                err << "... Attempting to reconnect " << Client.nick 
                    << " " << retryCount << "\n";
                win32_print(err.str);
                
                successCode = win32_try_connect_twitch(&Client);
                
                if (0 == successCode)
                {
                    isConnected = BOOL_TRUE;
                    break;
                }
                
                Sleep(5000);
            }
            
            if (isConnected)
            {
                win32_print("Successful Reconnect\n");
            }
            else
            {
                win32_print("Failed Reconnect, closing...\n");
                break;
            }
        } 
    }
    
    VirtualFree(receiveBuffer, 0, MEM_DECOMMIT | MEM_RELEASE);
    
    Close:
    
    WSACleanup();
    ExitProcess(0);
}