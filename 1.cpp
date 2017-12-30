#include <map>
#include "config.h"
using namespace std;

map<string, int> UserOL;
map<string, User *> UserInfo;
map<string, map<string, string> > offlinemsg;
map<string, map<string, string> > offlinedoc;
int total = 0;
pthread_mutex_t mutex;

void * handleRequest(void *fd) {

    char recvBuf[MAX_BUFFER];

    int psockfd = * ((int *) fd);
    printf("Thread sockfd: %d\n", psockfd);

    ssize_t size;
    Message receivemsg;
    Message sendmsg;

    while(true) {
        size = read(psockfd, &receivemsg, sizeof(Message));
        printf("Received %lu Bytes\n", size);

        /* 准备待发送的消息 */
        memset(&sendmsg, 0, sizeof(Message));

        if(receivemsg.type == LOGIN) {
            sendmsg.type = LOGIN;
            string temp = receivemsg.usrname;
            cout << "LOGIN Convert to string: " << temp << endl;
            if(UserInfo.find(temp) != UserInfo.end()) {
                cout << "login size: " << UserOL.size() << endl; 
                if(UserOL.find(temp) != UserOL.end()) {
                    sscanf("AlreadyOnline!", "%s", sendmsg.content);
                    printf("%s\n", sendmsg.content);
                }
                else
                    if(strcmp(receivemsg.password, UserInfo[temp]->password) == 0) {
                        sscanf("LoginSuccess!", "%s", sendmsg.content);
                        strcpy(sendmsg.usrname, receivemsg.usrname); 
                        printf("%s\n", sendmsg.content);
                        pthread_mutex_lock(&mutex);
                        UserInfo[temp]->login = true;
                        UserOL.insert(pair<string, int>(temp, psockfd));
                        pthread_mutex_unlock(&mutex);
                        printf("Add to Online User\n");
                    }
                    else {
                        sscanf("PasswordError!", "%s", sendmsg.content);
                        printf("%s\n", sendmsg.content);
                    }
            }
            else {
                sscanf("NoSuchUser!", "%s", sendmsg.content);
                printf("%s\n", sendmsg.content);
            }
            write(psockfd, &sendmsg, sizeof(Message));
        }
        else if(receivemsg.type == SIGNIN) {
            sendmsg.type = SIGNIN;
            string temp = receivemsg.usrname;
            cout << "SIGNIN Convert to string: " << temp << endl;
            if(UserInfo.find(temp) != UserInfo.end()) {
                sscanf("AlreadyExist!", "%s", sendmsg.content);
                printf("%s\n", sendmsg.content);
            }
            else { // 添加新用户 不登录
                User *newUsr = new User();
                newUsr->chatwith = NULL;
                strcpy(newUsr->name, receivemsg.usrname);
                strcpy(newUsr->password, receivemsg.password);
                newUsr->login = false;
                pthread_mutex_lock(&mutex);
                UserInfo.insert(pair<string, User *>(temp, newUsr));
                offlinemsg.insert(pair<string, map<string, string> >(temp, map<string, string>()));
                offlinedoc.insert(pair<string, map<string, string> >(temp, map<string, string>()));
                total += 1;
                pthread_mutex_unlock(&mutex);
                sscanf("SignInSuccess!", "%s", sendmsg.content);
                printf("%s\n", sendmsg.content);
            }
            write(psockfd, &sendmsg, sizeof(Message));
        }
        else if(receivemsg.type == CHAT) {
            string temp = receivemsg.usrname;
            string who = receivemsg.content;
            cout << "CHAT Convert to string: " << temp << endl;
            sendmsg.type = CHAT;
            if(UserInfo[who]->chatwith != NULL) {
                sscanf("Busy!", "%s", sendmsg.content);
                write(psockfd, &sendmsg, sizeof(Message));
            }
            else {
                if(UserInfo[who]->login == true) {
                    pthread_mutex_lock(&mutex);
                    UserInfo[temp]->chatwith = UserInfo[who];
                    UserInfo[who]->chatwith = UserInfo[temp];
                    pthread_mutex_unlock(&mutex);
                    string chat1 = temp + " chat with you";
                    string chat2 = who +  " chat with you";
                    strcpy(sendmsg.content, chat2.data());
                    write(psockfd, &sendmsg, sizeof(Message));
                    strcpy(sendmsg.content, chat1.data());
                    write(UserOL[who], &sendmsg, sizeof(Message));
                }
                else {
                    pthread_mutex_lock(&mutex);
                    UserInfo[temp]->chatwith = UserInfo[who];
                    pthread_mutex_unlock(&mutex);
                    string chat1 = who + " offline chat with you";
                    strcpy(sendmsg.content, chat1.data());
                    write(psockfd, &sendmsg, sizeof(Message));
                }
            }
        }
        else if(receivemsg.type == LS) {
            sendmsg.type = LS;
            write(psockfd, &sendmsg, sizeof(Message));
            string temp = receivemsg.usrname;
            int num = UserInfo[temp]->myfriend.size();
            printf("num : %d\n", num);
            char *totalinfo = new char[num * 24 + 4];
            memset((void *)totalinfo, 0, num * 24 + 4);
            *((int *) totalinfo) = 24 * num + 4;
            char * pos = totalinfo + 4;
            pthread_mutex_lock(&mutex);
            for(int i = 0; i < num; i++) {
                strcpy(pos, UserInfo[temp]->myfriend[i]->name);
                pos += 19;
                if(UserInfo[temp]->myfriend[i]->login == true) {
                    strcpy(pos, " ol\n");
                }
                else {
                    strcpy(pos, "   \n");
                }
                pos += 5;
            }
            send(psockfd, totalinfo, num * 24 + 4, MSG_WAITALL);
            pthread_mutex_unlock(&mutex);
            delete[] totalinfo;
        }
        else if(receivemsg.type == SENDMSG) {
            string temp = receivemsg.usrname;
            cout << "SENDMSG Convert to string: " << temp << endl;
            string who = UserInfo[temp]->chatwith->name;
            string cont = receivemsg.content;
            if(UserInfo[who]->login == true) {
                write(UserOL[who], &receivemsg, sizeof(Message));
                cout << "Comeheretrue" << endl;
            }
            else {
                pthread_mutex_lock(&mutex);
                map<string, string>::iterator iter;
                if((iter = offlinemsg[who].find(temp)) == offlinemsg[who].end()) {
                    cout << "First Insert" << endl;
                    cout << who << endl;
                    cout << temp << endl;
                    cout << cont << endl;
                    offlinemsg[who].insert(pair<string, string>(temp, cont));
                    cout << "count " << offlinemsg[who].size() << endl;
                }
                else {
                    iter->second += "\n";
                    iter->second += cont;
                    cout << iter->second;
                }
                pthread_mutex_unlock(&mutex);
            }
                
        }
        else if(receivemsg.type == RECVMSG) {
            string temp = receivemsg.usrname;
            cout << "RECVMSG" << temp << endl;;
            sendmsg.type = RECVMSG;
            write(psockfd, &sendmsg, sizeof(Message));
            string s;
            int length;
            cout << "sendcount " << offlinemsg[temp].size() << endl;
            pthread_mutex_lock(&mutex);
            map<string, string>::iterator iter;
            for(iter = offlinemsg[temp].begin(); iter != offlinemsg[temp].end(); iter++) {
                s = s + "\nFrom ";
                cout << iter->first;
                s = s + iter->first;
                s = s + "\n";
                cout << iter->second;
                s = s + iter->second;
                s = s + "\n";
            }
            pthread_mutex_unlock(&mutex);
            length = s.length();
            char *tosend = new char[4 + length + 1];
            *( (int *) tosend) = length;
            strcpy(tosend + 4, s.data());
            send(psockfd, tosend, 4 + length + 1, MSG_WAITALL);
            delete[] tosend; 
        }
        else if(receivemsg.type == RECVFILE) {
            string temp = receivemsg.usrname;
            cout << "Receive file of " << temp << endl;;
            sendmsg.type = RECVMSG;
            write(psockfd, &sendmsg, sizeof(Message));
            string s;
            int length;
            //cout << "sendcount " << offlinemsg[temp].size() << endl;
            pthread_mutex_lock(&mutex);
            map<string, string>::iterator iter;
            for(iter = offlinedoc[temp].begin(); iter != offlinedoc[temp].end(); iter++) {
                s = s + "\nFrom ";
                //cout << iter->first;
                s = s + iter->first;
                s = s + "\n";
                //cout << iter->second;
                s = s + iter->second;
                s = s + "\n";
            }
            pthread_mutex_unlock(&mutex);
            length = s.length();
            char *tosend = new char[4 + length + 1];
            *( (int *) tosend) = length;
            strcpy(tosend + 4, s.data());
            send(psockfd, tosend, 4 + length + 1, MSG_WAITALL);
            delete[] tosend; 
        }
        else if(receivemsg.type == SENDFILE) {
            int totallength = receivemsg.length;
            string temp = receivemsg.usrname;
            string who = UserInfo[temp]->chatwith->name;
            string filename = receivemsg.content;
            int length = 0;
            int temp1;
            FILE * fp = NULL;
            bool login = false;
            if(UserInfo[who]->login == true) {
                write(UserOL[who], &receivemsg, sizeof(Message));
                login = true;
            }
            else {
                fp = fopen(receivemsg.content, "wb");
                pthread_mutex_lock(&mutex);
                map<string, string>::iterator iter;
                if((iter = offlinedoc[who].find(temp)) == offlinedoc[who].end()) {
                    offlinedoc[who].insert(pair<string, string>(temp, filename));
                }
                else {
                    iter->second += "\n";
                    iter->second += filename;
                    cout << iter->second;
                }
                pthread_mutex_unlock(&mutex);
            }
            //int length = 0;
            //int temp1;
            while((temp1 = read(psockfd, recvBuf, MAX_BUFFER)) > 0) {
                length += temp1;
                printf("%d\n", length);
                if(login)
                    write(UserOL[who], recvBuf, temp1);
                else
                    fwrite(recvBuf, sizeof(char), temp1, fp);
                memset(recvBuf, 0, sizeof(recvBuf));
                if(length == totallength)
                    break;
            }

            fclose(fp);
        }
        else if(receivemsg.type == SEARCH) {
            sendmsg.type = SEARCH;
            write(psockfd, &sendmsg, sizeof(Message));
            string temp = receivemsg.usrname;
            cout << "SEARCH Convert to string: " << temp << endl;
            char *totalinfo = new char[24 * total + 4];
            memset((void *)totalinfo, 0, 24 * total + 4);
            *((int *) totalinfo) = 24 * total + 4;
            char * pos = totalinfo + 4;
            map<string, User *>::iterator iter;
            int index = 0;
            pthread_mutex_lock(&mutex);
            for(iter = UserInfo.begin(); iter != UserInfo.end(); iter++) {
                index++;
                printf("index: %d\n", index);
                if(temp == iter->first) continue;
                strcpy(pos, iter->second->name);
                pos += 19;
                if(iter->second->login == true) {
                    strcpy(pos, " ol\n");
                }
                else {
                    strcpy(pos, "   \n");
                }
                pos += 5;
            }
            pthread_mutex_unlock(&mutex);
            for(int i = 4; i < 24 * total + 4; i++) {
                putchar(totalinfo[i]);
            }
            send(psockfd, totalinfo, 24 * total + 4, MSG_WAITALL);
            delete[] totalinfo;
        }
        else if(receivemsg.type == LOGOUT) {
            sendmsg.type = LOGOUT;
            string temp = receivemsg.usrname;
            cout << "LOGOUT Convert to string: " << temp << endl;
            pthread_mutex_lock(&mutex);
            UserInfo[temp]->login = false;
            UserOL.erase(temp);
            pthread_mutex_unlock(&mutex);
            sscanf("LogoutSuccess!", "%s", sendmsg.content);
            printf("%s\n", sendmsg.content);
            write(psockfd, &sendmsg, sizeof(Message));
        }
        else if(receivemsg.type == QUIT) {
            sendmsg.type = QUIT;
            string temp = receivemsg.usrname;
            string who = UserInfo[temp]->chatwith->name;
            UserInfo[temp]->chatwith->chatwith = NULL;
            UserInfo[temp]->chatwith = NULL;
            string chat1 = temp + " off chat";
            string chat2 = who + " off chat";
            strcpy(sendmsg.content, chat2.data());
            write(psockfd, &sendmsg, sizeof(Message));
            strcpy(sendmsg.content, chat1.data());
            write(UserOL[who], &sendmsg, sizeof(Message));
        }
        else if(receivemsg.type == ADD) {
            sendmsg.type = ADD;
            string temp = receivemsg.usrname;
            string tofriend = temp + " have added your friend";
            string who = receivemsg.content;
            string tome = who + " added";
            pthread_mutex_lock(&mutex);
            UserInfo[temp]->myfriend.push_back(UserInfo[who]);
            strcpy(sendmsg.content, tofriend.data());
            write(UserOL[who], &sendmsg, sizeof(Message));
            strcpy(sendmsg.content, tome.data());
            write(psockfd, &sendmsg, sizeof(Message));
            pthread_mutex_unlock(&mutex);
        }
        else if(receivemsg.type == EXIT) {
            string temp = receivemsg.usrname;
            pthread_mutex_lock(&mutex);
            close(psockfd);
            UserOL.erase(temp);
            pthread_mutex_unlock(&mutex);
            break;
        }
    }
}


int main()
{
    //map<string, int> UserOL;
    //map<string, User*> UserInfo;
    pthread_mutex_init(&mutex, NULL);
    int numOL = 0;

    /* 配置服务器地址与客户端地址 */
    struct sockaddr_in serveraddr, clientaddr;
    socklen_t socklength = sizeof(clientaddr);

    /* 监听套接字标识符与通信套接字标识符 */
    int listenfd, resfd;

    /* 多线程处理多个用户 */
    pthread_t threads[MAX_CONNECTIONS];

    /* 创建监听套接字 */
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    memset((void *) &serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(PORT);

    /* 绑定 */
    if(bind(listenfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) {
        perror("bind error!");
        exit(1);
    }

    /* 开始监听 */
    if(listen(listenfd, MAX_CONNECTIONS) < 0) {
        perror("listen error!");
        exit(1);
    }

    //ssize_t size;

    while(true) {

        /* accept 阻塞等待链接 */
        if((resfd = accept(listenfd, (struct sockaddr *) &clientaddr, &socklength)) < 0) {
            perror("accept error!");
            exit(1);
        }

        printf("Accept a new connection!\n");
        printf("Connection from %s\n", inet_ntoa(clientaddr.sin_addr));

        pthread_create(&threads[numOL], NULL, handleRequest, (void *)&resfd);
        numOL += 1;
    }

    pthread_mutex_destroy(&mutex);
    return 0;
}