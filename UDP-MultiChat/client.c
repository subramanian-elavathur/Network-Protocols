/** Chat Client V0.2
  * Author:: E.V.Subramanian
  */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define SERVER_IP "127.0.0.1"
#define RPORT 6893
#define MAXBUFLEN 100
#define UIDLENLIMIT 50
#define MAXBUFLEN 1000

void getOperationFromRequest(char* senq, char* loc){
    char temp[3];
    int ptemp = 0;
    while(ptemp!=3){
        temp[ptemp] = senq[ptemp];
        ptemp++;
    }
    strcpy(loc,temp);
}

int main(int argc, char *argv[])
{
    int t_sockfd, r_sockfd;
    struct sockaddr_in their_addr;
    struct hostent *he;
    int numbytes, flushflag=0;
    char user[UIDLENLIMIT], op[5];
    char buf[MAXBUFLEN];
    char* buf2;
    char buf3[MAXBUFLEN];
    buf2 = (char*) malloc(sizeof(char)*MAXBUFLEN);
    int maxsize = MAXBUFLEN;
    int addr_len = sizeof(struct sockaddr);

    printf("Enter your Username - ");
    scanf("%s", user);

    char uidt[UIDLENLIMIT];
    sprintf(uidt, "-%s", user);

    if(fork()==0){
        printf("\nLISTENER:: Acquiring Socket\n");
        if ((r_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            perror("socket");
            exit(1);
        }

        printf("\nLISTENER:: Socket Acquired \n");
        printf("\nLISTENER:: Registering User Now \n");

        their_addr.sin_family = AF_INET;
        their_addr.sin_port = htons(RPORT);
        their_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
        bzero(&(their_addr.sin_zero), 8);

        char payload[1000] = "REG-";
        memcpy( &payload[4], user, strlen(user));
        payload[strlen(payload)] = '\0';
        //printf("\nLISTENER:: PAYLOAD - %s\n",payload);

        if ((numbytes=sendto(r_sockfd, payload, strlen(payload), 0,(struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
            perror("recvfrom");
            exit(1);
        }

        printf("\nLISTENER:: User Registration Request Sent awaiting reply\n");


        if ((numbytes=recvfrom(r_sockfd,buf, MAXBUFLEN-1, 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) {
            perror("recvfrom");
            exit(1);
        }

        if(strcmp(buf,"AFFIRM")==0){
            memset(&buf[0], 0, sizeof(buf));
            printf("\nLISTENER:: SERVER REGISTRATION COMPLETE\n");
            printf("\nLISTENER:: Initiating Listen\n");
            while(1==1){
                if ((numbytes=recvfrom(r_sockfd,buf, MAXBUFLEN-1, 0,(struct sockaddr *)&their_addr, &addr_len)) == -1) {
                    perror("recvfrom");
                    exit(1);
                }else{
                    if(strcmp(buf,"NEG")!=0){
                        printf("\n\n%s",buf);
                    }
                    if(strcmp(buf,"NEG")==0){
                        printf("\nLISTENER:: LOGOUT\n");
                        exit(0);
                    }
                    char header[7];
                    memcpy( header, buf, 6 );
                    header[6] = '\0';
                    if(strcmp(header,"ONLINE")!=0){
                        printf("\n\n");
                        printf("$: ");
                        fflush(stdout);
                    }
                    memset(&buf[0], 0, sizeof(buf));
                }
                memset(&buf[0], 0, sizeof(buf));
            }
        }
        else{
            printf("\nLISTENER:: ERROR REGISTERING WITH SERVER EXITING NOW\n");
            exit(0);
        }
    }
    else{
        if ((t_sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
            perror("socket");
            exit(1);
        }
        their_addr.sin_family = AF_INET;
        their_addr.sin_port = htons(RPORT);
        their_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
        bzero(&(their_addr.sin_zero), 8);
        sleep(2);
        printf("\n-------------------\nWelcome to the Chat Client V0.1\n-------------------\n");
        printf("\nFollowing are legal commands\n1.GET || gets all online users\n2.SEN-username-message || Sends message to username\n3.EXT || Logout\n");
        while(1==1){
            memset(&buf2[0], 0, sizeof(buf2));
            memset(&buf3[0], 0, sizeof(buf3));
            /** Chat Server V0.1
              * Author:: E.V.Subramanian
              * MICROSECONDS 20000 - gives a good UX
              */

            usleep(200000);
            printf("\n$: ");
            if(flushflag==0){
                int bytes_read = getline(&buf2, &maxsize, stdin);
                flushflag = 1;
            }
            int bytes_read = getline(&buf2, &maxsize, stdin);

            memcpy( buf3, buf2, strlen(buf2)-1);

            getOperationFromRequest(buf3, op);


            if(strcmp(op,"SEN")){
                memcpy( &buf3[strlen(buf3)], uidt, strlen(uidt));
            }

            if((numbytes=sendto(t_sockfd, buf3, strlen(buf3), 0,(struct sockaddr *)&their_addr, sizeof(struct sockaddr))) == -1) {
                printf("ERR:: SERVER ERROR EXITING NOW");
                exit(0);
            }

            if(strcmp(buf2,"EXT\n")==0){
                printf("\nCLIENT:: LOGGING OUT\n");
                printf("\nCLIENT:: CLEANING UP\n");
                sleep(2);
                printf("\nCLIENT:: THANK YOU FOR USING Chat Client\n\n");
                exit(0);
            }


        }

    }

    return 0;
}
