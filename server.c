#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BOARD_SIZE 3

typedef struct {
    char board[BOARD_SIZE][BOARD_SIZE];
    int currentPlayer;
    int numPlayers;
    int gameOver;
} GameState;

typedef struct {
    int socket;
    int playerId;
} Player;


void initializeGame() {
    memset(gameState.board, ' ', sizeof(gameState.board));
    gameState.currentPlayer = 0;
    gameState.numPlayers = 0;
    gameState.gameOver = 0;
}

void printBoard() {
    printf("Current board:\n");
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            printf("%c ", gameState.board[i][j]);
        }
        printf("\n");
    }
}


int isValidMove(int row, int col) {
    return row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE && gameState.board[row][col] == ' ';
}

int checkWinner() {
    // Verifica linhas e colunas
    for (int i = 0; i < BOARD_SIZE; i++) {
        if (gameState.board[i][0] != ' ' && gameState.board[i][0] == gameState.board[i][1] && gameState.board[i][1] == gameState.board[i][2])
            return 1;
        if (gameState.board[0][i] != ' ' && gameState.board[0][i] == gameState.board[1][i] && gameState.board[1][i] == gameState.board[2][i])
            return 1;
    }
    // Verifica diagonais
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



int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Criar o socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forçar a conexão a usar o porto
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Associar o socket
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Escutar por conexões
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Aguardando conexão na porta %d...\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        printf("Cliente conectado\n");
        // Lógica de processamento de cliente aqui
        close(new_socket);
    }

    return 0;
}
