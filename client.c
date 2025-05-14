#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

#define PORT 8090
#define CHUNK_SIZE 1024

struct RFTPPacket{
        int seq_num;
        int ack_num;
        int is_ack;
        int is_last;
        char data[CHUNK_SIZE];
};

int main(int argc, char *argv[]){
        if (argc != 2) {
                printf("Usage: %s <file_to_send>\n", argv[0]);
                return 1;
        }

        FILE *file = fopen(argv[1], "rb");
        if (!file) {
                perror("fopen");
                return 1;
        }

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

        if(inet_pton(AF_INET,"127.0.0.1",&server_addr.sin_addr)<0){
                perror("invalid address");
                exit(EXIT_FAILURE);
        }

        if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr))<0){
                perror("connect failed");
                exit(EXIT_FAILURE);
        }

        int seq = 0;
            struct RFTPPacket packet, ack;
            while (1) {
                memset(&packet, 0, sizeof(packet));
                size_t n = fread(packet.data, 1, CHUNK_SIZE, file);
                if (n <= 0) break;

                packet.seq_num = seq;
                packet.is_last = feof(file) ? 1 : 0;

                // Send packet
                sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr *)&server_addr,sizeof(server_addr));
                printf("Sent packet seq = %d\n", seq);

                // Wait for ACK
                recvfrom(sockfd, &ack, sizeof(ack), 0, NULL, NULL);
                if (ack.is_ack && ack.ack_num == seq + 1) {
                    printf("Received ACK for %d\n", ack.ack_num);
                    seq++;
                } else {
                    printf("Invalid ACK. Resending...\n");
                    // Implement retransmission here if needed
                }

                if (packet.is_last) break;
            }

            fclose(file);
            close(sockfd);
            return 0;
}