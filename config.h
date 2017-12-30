#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <string>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <pthread.h>

using namespace std;

const int PORT = 8888;
const int MAX_CONNECTIONS = 100;
const int MAX_LENGTH = 1024;   // 最大消息长度
const int MAX_BUFFER = 2048;   // 接收发送缓冲区大小

typedef struct User{
    char name[20];
    char password[20];
    bool login;
    vector<User *> myfriend;
    User * chatwith;
}User;

enum MessageType{
    SIGNIN = 1,
    LOGIN,
    SEARCH,
    LOGOUT,
    ADD,
    EXIT,
    LS,
    CHAT,
    SENDMSG,
    SENDFILE,
    QUIT,
    RECVMSG
};

typedef struct{
    char usrname[20];
    char password[20];
    int length;
    char content[MAX_LENGTH];
    MessageType type;
}Message;

