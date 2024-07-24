#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080


int main() {
    struct sockaddr_in serv_addr;
    int sock = 0;
    char buffer[1024] = {0};

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        printf("\nConnection Failed \n");
        return -1;
    }


    while (1) {
        int row, col;
        
        printf("Enter row and column (0-2): ");       
        scanf("%d %d", &row, &col);      
        
        memset(buffer, 0, sizeof(buffer)); 
        snprintf(buffer, sizeof(buffer), "%d %d", row, col);
        send(sock, buffer, strlen(buffer), 0); //Envio da linha e coluna    
                
        read(sock, buffer, 1024); //Recebe confirmacao de movimento valido       
        printf("%s\n", buffer);  
           
        //Exibe o board
        read(sock, buffer, 1024);
        printf("%s\n", buffer); 

        if (strcmp(buffer, "**** VOCE VENCEU!!! ****\n") == 0 || strcmp(buffer, "**** VOCE PERDEU!!! ****\n") == 0)
        {
            close(sock);
            return 0;
        }
        
    }

    close(sock);
    return 0;
}