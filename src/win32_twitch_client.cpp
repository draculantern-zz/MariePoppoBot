#include "win32_platform.h"
#include "win32_twitch_client.h"

#include "platform.h"
#include "drac_array.h"
#include "drac_random.h"
#include "drac_string.h"
#include "drac_memory.h"

#include "win32_platform.cpp"
#include "drac_random.cpp"
#include "twitch.cpp"
#include "highp/highp.cpp"

PlatformApi Platform;

#define RECEIVE_BUFFER_LENGTH (megabytes(1))

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
        return -1;
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
            return -1;
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
        return -1;
    }
    
    
    // login
    STACK_STRING(sendString, kilobytes(1));
    twitch_format_pass_message(client->pass, &sendString);
    
    int passResult =send(client->ircSocket, sendString.str, (int)sendString.length, 0);
    
    string_clear(&sendString);
    twitch_format_nick_message(client->nick, &sendString);
    
    int nickResult = send(client->ircSocket, sendString.str, (int)sendString.length, 0);
    
    if ((SOCKET_ERROR == passResult) || (SOCKET_ERROR == nickResult))
    {
        string_clear(&err);
        err << "Unable to login to twitch on account ("
            << client->nick << ")\n failed with error code:" << WSAGetLastError();
        win32_print(err.str);
        return -1;
    }
    
    string_clear(&sendString);
    twitch_format_join_message(client->channel, &sendString);
    
    int joinResult = send(client->ircSocket, sendString.str, (int)sendString.length, 0);
    
    if (SOCKET_ERROR == joinResult)
    {
        string_clear(&err);
        err << "Unable to connect to twitch channel ("
            << client->channel << ")\n failed with error code:" << WSAGetLastError();
        win32_print(err.str);
        return -1;
    }
    
    return 0;
}

FUNCTION void 
win32_twitch_wait()
{
    LOCAL_STATIC s64 perfFrequency = 0;
    if (!perfFrequency)
    {
        LARGE_INTEGER freqLargeInt;
        QueryPerformanceFrequency(&freqLargeInt);
        perfFrequency = freqLargeInt.QuadPart;
    }
    
    LARGE_INTEGER counterLargeInt;
    QueryPerformanceCounter(&counterLargeInt);
    s64 crntPerfCounter = counterLargeInt.QuadPart;
    
    f64 seconds = (f64)crntPerfCounter / perfFrequency;
    
    twitch_wait_to_not_get_banned(seconds);
}

FUNCTION
SEND_TWITCH_TEXT_CALLBACK(win32_send_text)
{
    win32_twitch_wait();
    Win32TwitchClient* win32Client = (Win32TwitchClient*)client->reserved;
    s32 result = send(win32Client->ircSocket, 
                      text, 
                      (int)textLength,
                      win32Client->sendFlags);
    
    if (SOCKET_ERROR == result)
    {
        return IRC_SOCKET_ERROR;
    }
    return TWITCH_SUCCESS;
}

FUNCTION
SEND_TWITCH_MESSAGE_CALLBACK(win32_send_message)
{
    win32_twitch_wait();
    Win32TwitchClient* win32Client = (Win32TwitchClient*)client->reserved;
    
    STACK_STRING(formattedMessage, TWITCH_MESSAGE_BUFFER);
    
    twitch_format_channel_message(twitchMessage, &formattedMessage);
    
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

int __stdcall
mainCRTStartup()
{
    win32_platform_init();
    
    //
    // Setup a console for the current Process 
    //
    HWND console = GetConsoleWindow();
    if (!console)
    {
        AllocConsole();
        console = GetConsoleWindow();
    }
    
    SetConsoleMode(console, 0);
    
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
    
    if (INVALID_HANDLE_VALUE == configFile)
    {
        win32_print("Could not find 'bot.config', what the heck\n");
        HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
        SetConsoleMode(in, 0);
        WaitForSingleObject(in, INFINITE);
        char buf[1];
        DWORD count;
        ReadConsole(in, buf, sizeof(buf), &count, NULL);
        ExitProcess(1);
    }
    
    DWORD configFileSize = GetFileSize(configFile, NULL);
    MemoryAllocation* configAllocation = win32_allocate_virtual_memory(megabytes(1), ALLOCATION_OVERFLOW_GUARD);
    char* configBuffer = (char*)configAllocation->ptr;
    
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
            
            if (match("nick", (char*)configBuffer + startConfigOption))
            {
                seek_next_char(&configReader, ':');
                s32 startConfigValue = seek_past_whitespace(&configReader);
                s32 endConfigValue = seek_next_whitespace(&configReader);
                configBuffer[endConfigValue] = 0;
                
                Client.nick = configBuffer + startConfigValue;
                to_lower(Client.nick);
            } 
            else if (match("pass", (char*)configBuffer + startConfigOption)) 
            {
                seek_next_char(&configReader, ':');
                s32 startConfigValue = seek_past_whitespace(&configReader);
                s32 endConfigValue = seek_next_whitespace(&configReader);
                configBuffer[endConfigValue] = 0;
                
                Client.pass = configBuffer + startConfigValue;
            } 
            else if (match("channel", (char*)configBuffer + startConfigOption)) 
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
    
    win32_free_virtual_memory(configAllocation);
    
    
    MemoryArena* stringMemory = allocate_memory_arena(kilobytes(64) + RECEIVE_BUFFER_LENGTH);
    String err = push_string(stringMemory, kilobytes(64));
    
    
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
    
    twitch_client_set_callback(Client.twitchClient,
                               SendTwitchMessage,
                               &win32_send_message);
    
    twitch_client_set_callback(Client.twitchClient,
                               SendText,
                               &win32_send_text);
    
    Client.twitchClient.arena = allocate_memory_arena(megabytes(1));
    
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
    
    char* receiveBuffer = (char*)push_size(stringMemory, RECEIVE_BUFFER_LENGTH);
    assert(receiveBuffer);
    
    const int recvFlags = 0;
    bool32 isConnected = BOOL_TRUE;
    
    // pump irc
    while (isConnected)
    {
        string_clear(&err);
        
        int nbytes = recv(Client.ircSocket, 
                          receiveBuffer,
                          RECEIVE_BUFFER_LENGTH - 1,
                          recvFlags);
        
        // msdn.microsoft.com/en-us/library/windows/desktop/ms740121(v=vs.85).aspx
        if (0 < nbytes)
        {
            receiveBuffer[nbytes] = 0;
            twitch_dispatch_callbacks(&Client.twitchClient, receiveBuffer, nbytes);
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