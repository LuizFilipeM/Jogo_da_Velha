#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define BOARD_SIZE 3


typedef struct {
    int socket;
    int playerId;
} Player;

typedef struct {
    char board[BOARD_SIZE][BOARD_SIZE];
    int currentPlayer;
    int numPlayers;
    int gameOver;
    Player list[2];
} GameState;

GameState gameState;
pthread_mutex_t gameStateMutex = PTHREAD_MUTEX_INITIALIZER;

void initializeGame() {
    memset(gameState.board, ' ', sizeof(gameState.board));
    gameState.currentPlayer = 0;
    gameState.numPlayers = 0;
    gameState.gameOver = 0;
}

void printBoard() {
    char board[6][6];
    printf("Current board:\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            board[i][j] = gameState.board[i][j];
        }
    }
}

int isValidMove(int row, int col) {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE && gameState.board[row][col] == ' ';
}

int checkWinner() {
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (gameState.board[i][0] != ' ' && gameState.board[i][0] == gameState.board[i][1] && gameState.board[i][1] == gameState.board[i][2])
            return 1;
        if (gameState.board[0][i] != ' ' && gameState.board[0][i] == gameState.board[1][i] && gameState.board[1][i] == gameState.board[2][i])
            return 1;
    }
    if (gameState.board[0][0] != ' ' && gameState.board[0][0] == gameState.board[1][1] && gameState.board[1][1] == gameState.board[2][2])
        return 1;
    if (gameState.board[0][2] != ' ' && gameState.board[0][2] == gameState.board[1][1] && gameState.board[1][1] == gameState.board[2][0])
        return 1;
    return 0;
}

void makeMove(int row, int col) {
    gameState.board[row][col] = (gameState.currentPlayer == 0) ? 'X' : 'O';
    if (checkWinner()) {
        gameState.gameOver = 1;
    } else {
        gameState.currentPlayer = (gameState.currentPlayer + 1) % gameState.numPlayers;
    }
}

void *handleClient(void *arg) {
    Player *player = (Player *)arg;
    char buffer[1024] = {0};
    int row, col;

    while (!gameState.gameOver) {
        int index = 0;
           
        

        int valread = read(player->socket, buffer, 1024); //Recebe linha e coluna
        
        if (valread > 0) {
            
            sscanf(buffer, "%d %d", &row, &col);
            pthread_mutex_lock(&gameStateMutex);
            if (gameState.currentPlayer == player->playerId && isValidMove(row, col)) {               
                makeMove(row, col);    
                memset(buffer, 0, sizeof(buffer));     
                snprintf(buffer, sizeof(buffer), "Movimento valido\n");
            } else {
                snprintf(buffer, sizeof(buffer), "Movimento invalido\n");
            }
            pthread_mutex_unlock(&gameStateMutex);
            
            send(player->socket, buffer, strlen(buffer), 0); //Envio validação de movimento
        }    
        
                            
        if (gameState.gameOver == 1)
        {
            snprintf(buffer, sizeof(buffer), "**** VOCE VENCEU!!! ****\n");
            send(gameState.list[gameState.currentPlayer].socket, buffer, strlen(buffer), 0);

            snprintf(buffer, sizeof(buffer), "**** VOCE PERDEU!!! ****\n");
            send(gameState.list[(gameState.currentPlayer + 1) % 2].socket, buffer, strlen(buffer), 0);

            close(player->socket);
            free(player);  
            return NULL;
        }


        memset(buffer, 0, sizeof(buffer));
        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                index += snprintf(buffer + index, sizeof(buffer) - index, " %c", gameState.board[i][j]);
                if (j < 3 - 1) {
                    index += snprintf(buffer + index, sizeof(buffer) - index, " |");
                }
            }
            index += snprintf(buffer + index, sizeof(buffer) - index, "\n");
            if (i < 3 - 1) {
                index += snprintf(buffer + index, sizeof(buffer) - index, "---+---+---\n");
                }   
            }
        
        printf("%s\n", buffer);
        
        send(player->socket, buffer, strlen(buffer), 0);


    }
    
    close(player->socket);
    free(player);    
    
    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    pthread_t thread_id;


    initializeGame();

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Aguardando conexões na porta %d...\n", PORT);


    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("Cliente conectado:\n");


        Player *player = malloc(sizeof(Player));
        player->socket = new_socket;
        player->playerId = gameState.numPlayers++;
        gameState.list[gameState.numPlayers-1] = *player;
        

        pthread_create(&thread_id, NULL, handleClient, (void *)player);
        
    }

    return 0;
}