#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<unistd.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main(){
    int sock=0;
    struct sockaddr_in serv_addr;

    char buffer[BUFFER_SIZE];
    char reg_no[BUFFER_SIZE];

    sock = socket(AF_INET, SOCK_STREAM, 0);

    // 2. Define server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);


    connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));


while (1)
{
    //user input
    printf("\n Enter reg no:");

    fgets(reg_no, BUFFER_SIZE, stdin);
    reg_no[strcspn(reg_no, "\n")]=0;


        if(strcmp(reg_no, "Bye")==0){
        printf("Conection closed.\n");
        break;
    }

    send(sock, reg_no, strlen(reg_no), 0);


    int n = read(sock, buffer, sizeof(buffer) - 1);

 if (n <= 0) {
     printf("Server disconnected.\n");
     break;
 }


 buffer[n] = '\0';

    printf("Server response: %s\n", buffer);



}

close(sock);

}
