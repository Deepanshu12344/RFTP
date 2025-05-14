#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 8090

struct RFTPPacket{
        int seq_num;
        int ack_num;
        int is_ack;
        int is_last;
        char data[1024];
};

int main(int argc,char *argv[]){
        int sockfd;
        struct sockaddr_in server_addr,client_addr;
        socklen_t client_len = sizeof(client_addr);

        sockfd=socket(AF_INET,SOCK_DGRAM,0);
        if(sockfd<0){
                perror("socket failed");
                exit(EXIT_FAILURE);
        }

        memset(&server_addr,0,sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(PORT);
        server_addr.sin_addr.s_addr = INADDR_ANY;

        if(bind(sockfd,(struct sockaddr*)&server_addr,sizeof(server_addr))<0){
                perror("bind failed");
                exit(EXIT_FAILURE);
        }

        printf("listening on port %d\n",PORT);

        struct RFTPPacket packet;
        while(1){
                int bytes = recvfrom(sockfd,&packet,sizeof(packet),0,(struct sockaddr*)&client_addr,&client_len);
                if(bytes>0){
                        printf("Received packet: Seq = %d, ACK = %d, is_ack = %d, is_last = %d\n",packet.seq_num,packet.ack_num,packet.is_ack,packet.is_last);
                        printf("Data: %s\n",packet.data);

                        struct RFTPPacket ack = {0};
                        ack.seq_num = 0;
                        ack.ack_num = packet.seq_num + 1;
                        ack.is_ack = 1;

                        sendto(sockfd, &ack, sizeof(ack), 0,(struct sockaddr *)&client_addr, client_len);
                }
        }


}