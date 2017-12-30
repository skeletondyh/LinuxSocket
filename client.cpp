#include "config.h"

bool login = false;
bool chat = false;
char localname[20];
int sockfd;
pthread_mutex_t mutex;


void * recvMsg(void *) {

    Message recvmsg;
    char recvBuf[MAX_BUFFER];

    //bool login = false;
    //bool chat = false;

    while(true) {
        //pthread_mutex_lock(&mutex);
        read(sockfd, &recvmsg, sizeof(Message));

        if(recvmsg.type == LOGIN) {
            printf("FromServer: %s\n", recvmsg.content);
            //pthread_mutex_lock(&mutex);
            if(strcmp(recvmsg.content, "LoginSuccess!") == 0) {
                //pthread_mutex_lock(&mutex);
                login = true;
                memset(localname, 0, sizeof(localname));
                strcpy(localname, recvmsg.usrname);
            }
        }
        else if(recvmsg.type == SIGNIN) {
            printf("FromServer: %s\n", recvmsg.content);
        }
        else if(recvmsg.type == SEARCH) {
            int totalsize = 0;
            int temp;
            char *Buf;
            if((temp = read(sockfd, recvBuf, MAX_BUFFER)) > 4) {
                totalsize = *((int *)recvBuf);
                Buf = new char[totalsize];
                memcpy(Buf, recvBuf, temp);
            }
            while(temp < totalsize) {
                temp += read(sockfd, Buf + temp, MAX_BUFFER);
            }
            for(int i = 4; i < totalsize; i++) {
                putchar(Buf[i]);
            }
            delete[] Buf;
        }
        else if(recvmsg.type == LOGOUT) {
            //read(sockfd, receivebuf, MAX_LENGTH);
            printf("FromServer: %s\n", recvmsg.content);
            //pthread_mutex_lock(&mutex);
            login = false;
            //memset(localname, 0, sizeof(localname));
            //pthread_mutex_unlock(&mutex);
        }
        else if(recvmsg.type == ADD) {
            printf("FromServer: %s\n", recvmsg.content);
        }
        else if(recvmsg.type == CHAT) {
            printf("FromServer: %s\n", recvmsg.content);
        }
        else if(recvmsg.type == RECVMSG || recvmsg.type == RECVFILE) {
            memset(recvBuf, 0, sizeof(recvBuf));
            int totalsize = 0;
            int temp;
            char *Buf;
            if((temp = read(sockfd, recvBuf, MAX_BUFFER)) > 4) {
                totalsize = *((int *)recvBuf);
                Buf = new char[totalsize];
                memcpy(Buf, recvBuf, temp);
                memset(recvBuf, 0, sizeof(recvBuf));
            }
            while(temp < totalsize) {
                temp += read(sockfd, recvBuf, MAX_BUFFER);
            }
            printf("%s", Buf + 4);
        }
        else if(recvmsg.type == SENDMSG) {
            printf("From%s: %s\n", recvmsg.usrname, recvmsg.content);
        }
        else if(recvmsg.type == QUIT) {
            printf("FromServer: %s\n", recvmsg.content);
        }
        else if(recvmsg.type == SENDFILE) {
            int totallength = recvmsg.length;
            printf("From%s: %s\n", recvmsg.usrname, recvmsg.content);
            FILE * fp = fopen(recvmsg.content, "wb");
            //fseek(fp, 0, SEEK_END);
            //int length = ftell(fp);
            int recvlength = 0;
            int temp;
            while((temp = read(sockfd, recvBuf, MAX_BUFFER)) > 0) {
                //printf("temp: %d\n", temp);
                recvlength += temp;
                printf("%d\n", recvlength);
                fwrite(recvBuf, sizeof(char), temp, fp);
                memset(recvBuf, 0, sizeof(recvBuf));
                if(recvlength == totallength)
                    break;
                //fflush(fp);
            }
            fclose(fp);
        }
        else if(recvmsg.type == LS) {
            int totalsize = 0;
            int temp;
            char *Buf;
            if((temp = read(sockfd, recvBuf, MAX_BUFFER)) > 4) {
                totalsize = *((int *)recvBuf);
                Buf = new char[totalsize];
                memcpy(Buf, recvBuf, temp);
            }
            while(temp < totalsize) {
                temp += read(sockfd, Buf + temp, MAX_BUFFER);
            }
            for(int i = 4; i < totalsize; i++) {
                putchar(Buf[i]);
            }
            delete[] Buf;
        }

        //pthread_mutex_lock(&mutex);
        /*if(login && !chat) {
            printf("ChatRoom >> ");
        }*/
    }
}

int main(int argc, char *argv[])
{

    pthread_mutex_init(&mutex, NULL);

    //int login = 0;
    //char localname[20];

    /* 套接字标识符与地址 */
    //int sockfd;
    struct sockaddr_in clientaddr;

    /* 缓冲 */
    char cmd[MAX_LENGTH];
    char sendbuf[MAX_BUFFER];
    //char receivebuf[MAX_BUFFER];

    /* 创建套接字 */
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket error!");
        exit(1);
    }

    memset((void *) &clientaddr, 0, sizeof(clientaddr));
    clientaddr.sin_family = AF_INET;
    clientaddr.sin_port = htons(PORT);
    if(inet_pton(AF_INET, argv[1], &clientaddr.sin_addr) < 0) {
        perror("inet_pton error!");
        exit(1);
    }

    /* 连接 */
    if(connect(sockfd, (struct sockaddr *) &clientaddr, sizeof(clientaddr)) < 0) {
        perror("connect error!");
        exit(1);
    }

    printf("Connection Success!\n");

    /* 待发和待收消息 */
    pthread_t thread;
    Message sendmsg;
    //Message receivemsg;
    pthread_create(&thread, NULL, recvMsg, NULL);

    while(true) {
        /*  读入命令 */
        scanf("%s", cmd);

        if(strcmp(cmd, "exit") == 0) {
            memset((void *)&sendmsg, 0, sizeof(Message));
            //scanf(" %s %s", sendmsg.usrname, sendmsg.password);
            strcpy(sendmsg.usrname, localname);
            sendmsg.type = EXIT;
            write(sockfd, &sendmsg, sizeof(Message));
            break;
        }

        if(strcmp(cmd, "login") == 0) {
            memset((void *)&sendmsg, 0, sizeof(Message));
            scanf(" %s %s", sendmsg.usrname, sendmsg.password);
            sendmsg.type = LOGIN;
            write(sockfd, &sendmsg, sizeof(Message));
        }
        else if(strcmp(cmd, "signin") == 0) {
            memset((void *)&sendmsg, 0, sizeof(Message));
            scanf(" %s %s", sendmsg.usrname, sendmsg.password);
            sendmsg.type = SIGNIN;
            write(sockfd, &sendmsg, sizeof(Message));
        }
        else if(strcmp(cmd, "recvmsg") == 0) {
            memset((void *) &sendmsg, 0, sizeof(Message));
            sendmsg.type = RECVMSG;
            strcpy(sendmsg.usrname, localname);
            write(sockfd, &sendmsg, sizeof(Message));
        }
        else if(strcmp(cmd, "recvfile") == 0) {
            memset((void *) &sendmsg, 0, sizeof(Message));
            sendmsg.type = RECVFILE;
            strcpy(sendmsg.usrname, localname);
            write(sockfd, &sendmsg, sizeof(Message));
        }
        else
            if(!login) {
                printf("You haven't logged in!\n");
            }
            else {
                if(strcmp(cmd, "search") == 0) {
                    memset((void *) &sendmsg, 0, sizeof(Message));
                    sendmsg.type = SEARCH;
                    strcpy(sendmsg.usrname, localname);
                    write(sockfd, &sendmsg, sizeof(Message));
                    /*int totalsize = 0;
                    int temp;
                    char *Buf;
                    if((temp = read(sockfd, receivebuf, MAX_BUFFER)) > 4) {
                        totalsize = *((int *)receivebuf);
                        Buf = new char[totalsize];
                        memcpy(Buf, receivebuf, temp);
                    }
                    while(temp < totalsize) {
                        temp += read(sockfd, Buf + temp, MAX_BUFFER);
                    }
                    for(int i = 4; i < totalsize; i++) {
                        putchar(Buf[i]);
                    }
                    delete[] Buf;*/
                }
                else if(strcmp(cmd, "logout") == 0) {
                    memset((void *) &sendmsg, 0, sizeof(Message));
                    sendmsg.type = LOGOUT;
                    strcpy(sendmsg.usrname, localname);
                    write(sockfd, &sendmsg, sizeof(Message));
                    /*read(sockfd, receivebuf, MAX_LENGTH);
                    printf("%s\n", receivebuf);
                    login = false;
                    memset(localname, 0, sizeof(localname));*/
                }
                else if(strcmp(cmd, "add") == 0) {
                    memset((void *) &sendmsg, 0, sizeof(Message));
                    sendmsg.type = ADD;
                    strcpy(sendmsg.usrname, localname);
                    scanf("%s", sendmsg.content);
                    write(sockfd, &sendmsg, sizeof(Message));
                }
                else if(strcmp(cmd, "ls") == 0) {
                    memset((void *) &sendmsg, 0, sizeof(Message));
                    sendmsg.type = LS;
                    strcpy(sendmsg.usrname, localname);
                    write(sockfd, &sendmsg, sizeof(Message));
                }
                else if(strcmp(cmd, "chat") == 0) {
                    memset((void *) &sendmsg, 0, sizeof(Message));
                    sendmsg.type = CHAT;
                    strcpy(sendmsg.usrname, localname);
                    scanf("%s", sendmsg.content);
                    write(sockfd, &sendmsg, sizeof(Message));
                }
                else if(strcmp(cmd, "sendmsg") == 0) {
                    memset((void *) &sendmsg, 0, sizeof(Message));
                    sendmsg.type = SENDMSG;
                    strcpy(sendmsg.usrname, localname);
                    scanf("%s", sendmsg.content);
                    write(sockfd, &sendmsg, sizeof(Message));
                }
                else if(strcmp(cmd, "quit") == 0) {
                    memset((void *) &sendmsg, 0, sizeof(Message));
                    sendmsg.type = QUIT;
                    strcpy(sendmsg.usrname, localname);
                    write(sockfd, &sendmsg, sizeof(Message));
                }
                else if(strcmp(cmd, "sendfile") == 0) {
                    memset((void *) &sendmsg, 0, sizeof(Message));
                    sendmsg.type = SENDFILE;
                    strcpy(sendmsg.usrname, localname);
                    scanf("%s", sendmsg.content);
                    FILE *fp = fopen(sendmsg.content, "rb");
                    if(fp == NULL) {
                        perror("No such file!");
                    }
                    else {
                        int totallength;
                        fseek(fp, 0, SEEK_END);
                        totallength = ftell(fp);
                        fseek(fp, 0, SEEK_SET);
                        printf("totalsize: %d\n", totallength);
                        sendmsg.length = totallength;
                        write(sockfd, &sendmsg, sizeof(Message));
                        int length = 0;
                        int temp;
                        while((temp = fread(sendbuf, sizeof(char), MAX_BUFFER, fp)) > 0) {
                            length += temp;
                            printf("%d\n", length);
                            write(sockfd, sendbuf, temp);
                            memset(sendbuf, 0, sizeof(sendbuf));
                            if(length == totallength)
                                break;
                        }
                        fclose(fp);
                    }
                }
                /*else if(strcmp(cmd, "recvmsg") == 0) {
                    memset((void *) &sendmsg, 0, sizeof(Message));
                    sendmsg.type = RECVMSG;
                    strcpy(sendmsg.usrname, localname);
                    write(sockfd, &sendmsg, sizeof(Message));
                }*/
            }
        
    }



    pthread_mutex_destroy(&mutex);
    close(sockfd);

    return 0;
}