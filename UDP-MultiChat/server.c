/** Chat Server V0.2
  * Author:: E.V.Subramanian
  */


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>

#define RPORT 6893
#define TPORT 16893
#define SERVER_IP "127.0.0.1"
#define USERLIMIT 100
#define MAXBUFLEN 1000
#define UIDLENLIMIT 50

int regtrack = 0;
struct sockaddr_in their_addr[USERLIMIT];
char uid[USERLIMIT][MAXBUFLEN];
int valid[USERLIMIT];

int validityCheck(char* in){
    if(strlen(in)<4){
        printf("\nERR: Invalid Command\n");
        return 0;
    }
    if(in[3]!='-'){
        printf("\nERR: Invalid Format\n");
        return 0;
    }

    char header[4];
    memcpy( header, in, 3 );
    header[3] = '\0';

    if(strcmp("REG",header)!=0 && strcmp("SEN",header)!=0 && strcmp("GET",header)!=0 && strcmp("EXT",header)!=0){
        printf("\nERR: Command Not Supported\n");
        return 0;
    }

    return 1;
}

void registerUser(char* in, struct sockaddr_in data){
    char header[4];
    memcpy( header, in, 3 );
    header[3] = '\0';

    if(strcmp("REG",header)!=0){
        printf("\nERR: Invalid Operation Requested\n");
        return 0;
    }

    char user[UIDLENLIMIT];
    memcpy( user, &in[4], strlen(in) - 4 );
    user[strlen(in)-4] = '\0';

    //data.sin_addr.s_addr = inet_addr("192.168.1.1");

    if(regtrack<USERLIMIT){
        strcpy (uid[regtrack], user);
        their_addr[regtrack] = data;

        int temp_socket,numbytes;

        if ((temp_socket = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            perror("socket");
            exit(1);
        }

        if ((numbytes=sendto(temp_socket, "AFFIRM", 6, 0,(struct sockaddr *)&their_addr[regtrack], sizeof(struct sockaddr))) == -1) {
            perror("recvfrom");
            exit(1);
        }

        valid[regtrack] = 1;
        regtrack++;
    }
    else{
        regtrack = 0;
        strcpy (uid[regtrack], user);

        // sended NEG from server to client

        their_addr[regtrack] = data;
        valid[regtrack] = 1;
        regtrack++;
    }
    printf("\nSERVER: OK User - %s - Registered || Regtrack of user is now - %d\n",uid[regtrack-1], regtrack-1);
    printf("\nSERVER: OK User - %s - IP - %s - PORT - %d\n",uid[regtrack-1], inet_ntoa(their_addr[regtrack-1].sin_addr), ntohs(their_addr[regtrack-1].sin_port));
}

void getMessageFromRequest(char* senq, char* target){
    printf("MASS:: %s\n", senq);
    char header[4];
    memcpy( header, senq, 3 );
    header[3] = '\0';

    if(strcmp("SEN",header)!=0){
        printf("\nERR: Invalid Operation Requested\n");
        return 0;
    }

    char temp[MAXBUFLEN] = "";
    memset(&temp[0], 0, sizeof(temp));
    int ptemp = 0, psenq = 4;
    while(psenq<strlen(senq)){
        if(senq[psenq]=='-')
            break;
        psenq++;
    }
    psenq++;
    while(psenq<strlen(senq)){
        if(senq[psenq]=='-')
            break;
        temp[ptemp] = senq[psenq];
        ptemp++;psenq++;
    }
    strcpy(target,temp);
}

void getSenderFromRequest(char* senq, char* target){
    char header[4];
    memcpy( header, senq, 3 );
    header[3] = '\0';

    if(strcmp("SEN",header)!=0){
        printf("\nERR: Invalid Operation Requested\n");
        return 0;
    }

    char temp[MAXBUFLEN] = "";
    memset(&temp[0], 0, sizeof(temp));
    int ptemp = 0, psenq = 4;
    while(psenq<strlen(senq)){
        if(senq[psenq]=='-')
            break;
        psenq++;
    }
    psenq++;
    while(psenq<strlen(senq)){
        if(senq[psenq]=='-')
            break;
        psenq++;
    }
    psenq++;
    while(psenq<strlen(senq)){
        if(senq[psenq]=='\0')
            break;
        temp[ptemp] = senq[psenq];
        ptemp++;psenq++;
    }
    char utemp[UIDLENLIMIT];
    memset(&utemp[0], 0, sizeof(utemp));
    memcpy( utemp, temp, strlen(temp) );
    strcpy(target,utemp);
}


char* getUIDFromRequest(char* senq){
    //printf("UID:: %s\n",senq);
    char header[4];
    memcpy( header, senq, 3 );
    header[3] = '\0';
    //printf("UID:: %s|\n",header);
    if(strcmp("SEN",header)!=0 && strcmp("GET",header)!=0 && strcmp("EXT",header)!=0){
        printf("\nERR: Invalid Operation Requested\n");
        return 0;
    }

    char temp[UIDLENLIMIT+5];
    memset(&temp[0], 0, sizeof(temp));
    int ptemp = 0, psenq = 4;
    while(psenq<strlen(senq)){
        if(senq[psenq]=='-')
            break;
        temp[ptemp] = senq[psenq];
        ptemp++; psenq++;
    }
    //printf("UID:: %s\n",temp);
    return temp;
}

void getOperationFromRequest(char* senq, char* loc){
    char temp[3];
    int ptemp = 0;
    while(ptemp!=3){
        temp[ptemp] = senq[ptemp];
        ptemp++;
    }
    printf("\nFOUND OP:: %s\n", temp);
    strcpy(loc,temp);
}

struct sockaddr_in getUserAddr(char* in){
    //printf("ADDR:: %s\n",in);
    int i;
    for(i = 0; i < USERLIMIT; i++)
        if(strcmp(in, uid[i])==0)
            return their_addr[i];
    printf("\nERR: User Not Registered\n");
}

struct sockaddr_in getAddrFromIp(uint32_t ip){
    int i;
     for(i = 0; i < USERLIMIT; i++)
        if(ip == their_addr[i].sin_addr.s_addr)
            return their_addr[i];
     printf("\nERR: User Not Registered\n");
}


/*
    test
    struct sockaddr_in test;
    printf("%d",validityCheck("REG$"));
    registerUser("REG$pikle6",test);
    struct sockaddr_in temp = getUserAddr(getUIDfromRequest("SEN$pikle6$hi this is nice"));
    printf("\n%s\n",inet_ntoa(temp.sin_addr));
    printf("\n%s\n",getOperationFromRequest("SEN$pikle6$hi this is nice"));
    printf("\n%s\n",getMessagefromRequest("SEN$pikle6$hi this is nice"));
*/


int main(void)
{
    int r_sockfd, t_sock_fd;
    struct sockaddr_in my_addr;
    struct sockaddr_in their_addr_temp; // connector’s address information
    int addr_len, numbytes;
    char buf[MAXBUFLEN];

    printf("\nListener:: Acquiring socket\n");

    if ((r_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("socket");
        exit(1);
    }
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(RPORT);
    my_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    bzero(&(my_addr.sin_zero), 8); // zero the rest of the struct

    if (bind(r_sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1) {
        perror("bind");
        exit(1);
    }

    addr_len = sizeof(struct sockaddr);

    printf("\nListener:: Started\n");
    while(1==1){
        if ((numbytes=recvfrom(r_sockfd,buf, MAXBUFLEN-1, 0,(struct sockaddr *)&their_addr_temp, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }
        else{
            char subbuff[15], op[5];
            memcpy( subbuff, &buf, numbytes );
            subbuff[numbytes] = '\0';
            printf("\nRAW:: %s - ip - %s port - %d\n",subbuff,inet_ntoa(their_addr_temp.sin_addr), ntohs(their_addr_temp.sin_port));
            if(validityCheck(subbuff)){
                getOperationFromRequest(subbuff, op);
                if(strcmp(op,"REG")==0){
                        printf("\nSERVER:: Registering new user\n");
                        registerUser(subbuff, their_addr_temp);
                        continue;
                }
                else if(strcmp(op,"SEN")==0){
                    printf("\nSERVER:: RELAYING MESSAGE NOW\n");
                    printf("\nSERVER:: subbuf - %s\n",subbuff);

                    int temp_sock;

                    if ((temp_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
                        perror("socket");
                        exit(1);
                    }

                    char mess[MAXBUFLEN], sender[UIDLENLIMIT], payl[MAXBUFLEN];

                    //THE BELOW LINE IS NEEDED IN LAB
                    //memset(&mess[0], 0, sizeof(mess)); memset(&sender[0], 0, sizeof(sender));

                    getMessageFromRequest(subbuff, mess);

                    printf("\n%s:: MESSAGE %s\n", sender, mess);

                    getSenderFromRequest(subbuff, sender);

                    sprintf(payl,"%s:: %s",sender, mess);

                    printf("\n%s:: MESSAGE %s\n", sender, mess);


                    struct sockaddr_in taddr = getUserAddr(getUIDFromRequest(subbuff));

                     if ((numbytes = sendto(temp_sock, payl, strlen(payl), 0,(struct sockaddr *)&taddr, sizeof(struct sockaddr))) == -1) {
                        perror("recvfrom");
                        exit(1);
                      }

                    printf("\SERVER:: MESSAGE SENT\n");

                    close(temp_sock);

                }

                /** Chat Server V0.2
                  * Author:: E.V.Subramanian
                  * EDIT:: added get and ext
                  * need to perform server side cleanup too
                  */


                else if(strcmp(op,"GET")==0){

                    int temp_sock;

                    if ((temp_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
                        perror("socket");
                        exit(1);
                    }

                    char mess[MAXBUFLEN];
                    memset(&mess[0], 0, sizeof(mess));
                    int i;
                    sprintf(mess,"ONLINE:: ");
                    for(i=0;i<regtrack;i++)
                        sprintf(&mess[strlen(mess)],"%s ,",uid[i]);
                    sprintf(&mess[strlen(mess)],"\n");

                    printf("\n%s\n", mess);


                    struct sockaddr_in taddr = getUserAddr(getUIDFromRequest(subbuff));

                     if ((numbytes = sendto(temp_sock, mess, strlen(mess), 0,(struct sockaddr *)&taddr, sizeof(struct sockaddr))) == -1) {
                        perror("recvfrom");
                        exit(1);
                      }

                    printf("\SERVER:: MESSAGE SENT\n");

                    close(temp_sock);
                }
                else if(strcmp(op,"EXT")==0){

                    int temp_sock;

                    if ((temp_sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
                        perror("socket");
                        exit(1);
                    }

                    struct sockaddr_in taddr = getUserAddr(getUIDFromRequest(subbuff));

                    if ((numbytes = sendto(temp_sock, "NEG", 3, 0,(struct sockaddr *)&taddr, sizeof(struct sockaddr))) == -1) {
                        perror("recvfrom");
                        exit(1);
                    }

                    printf("\nSERVER:: MESSAGE SENT\n");

                    close(temp_sock);
                }
                else{
                    printf("\SERVER:: No Such Operation\n");
                }
          }
        }
    }


    close(r_sockfd);
    return 0;
}
