#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUFFER_SIZE 4096

void parse_url(const char *url, char *host, int *porta, char *caminho) {
    char temp[256];
    strncpy(temp, url, sizeof(temp)-1);
    temp[sizeof(temp)-1] = '\0';

    char *p = temp;

    // Remove "http://"
    if(strncmp(p, "http://", 7) == 0) p += 7;

    *porta = 80;  // porta padrão

    char *slash = strchr(p, '/');
    if(slash){
        strcpy(caminho, slash + 1);
        *slash = '\0';
    }else{
        caminho[0] = '\0';
    }

    char *colon = strchr(p, ':');
    if(colon){
        *colon = '\0';
        *porta = atoi(colon + 1);
    }

    strcpy(host, p);

    if(strlen(caminho) == 0)
        strcpy(caminho, "index.html");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Use: %s http://host:porta/arquivo\n", argv[0]);
        return 0;
    }

    char host[128], caminho[128] = "";
    int porta = 80; // padrão HTTP

    parse_url(argv[1], host, &porta, caminho);

    printf("Conectando em %s:%d\n", host, porta);
    printf("Recurso solicitado: /%s\n", caminho);

    struct hostent *server = gethostbyname(host);
    if(!server){
        printf("Erro: host nao encontrado (%s)\n", host);
        return 0;
    }

    // Criação do socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        printf("Erro ao criar socket");
        return 0;
    }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(porta);
    memcpy(&serv_addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);

    // Conectando ao servidor
    if(connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("Erro ao conectar ao servidor");
        close(sock);
        return 0;
    }

    // Enviando reqwuisição
    char requisicao[512];
    snprintf(requisicao, sizeof(requisicao),"GET /%s HTTP/1.0\r\n" "Host: %s\r\n" "Connection: close\r\n\r\n", caminho, host);
    write(sock, requisicao, strlen(requisicao));
    printf("Requisicao enviada:\n%s\n", requisicao);

    // Extrai o nome do arquivo a partir do caminho da URL
    char *nome_arquivo = strrchr(caminho, '/');
    if(nome_arquivo) nome_arquivo++; // pega somente o nome após a última '/'
    else nome_arquivo = caminho;     // se não houver '/', usa todo o caminho

    FILE *saida = fopen(nome_arquivo, "wb");
    if(!saida){
        printf("Erro ao criar arquivo local\n");
        close(sock);
        return 0;
    }

    char buffer[BUFFER_SIZE];
    int bytes_lidos; //quantos bytes read() retornou
    int header_lido = 0; // indica se já encontramos e descartamos o cabeçalho

    while((bytes_lidos = read(sock, buffer, sizeof(buffer))) > 0){
        if(!header_lido){
            char *corpo = strstr(buffer, "\r\n\r\n");
            if(corpo){
                header_lido = 1;
                corpo += 4; // pula os "\r\n\r\n"
                int tamanho_cabecalho = corpo - buffer;
                int tamanho_corpo = bytes_lidos - tamanho_cabecalho;
                fwrite(corpo, 1, tamanho_corpo, saida);
            }
        }else{
            fwrite(buffer, 1, bytes_lidos, saida);
        }
    }

    printf("Download concluído. Arquivo salvo como '%s'\n", nome_arquivo);

    fclose(saida);
    close(sock);
    return 0;
}
