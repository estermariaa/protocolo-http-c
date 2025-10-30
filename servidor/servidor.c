#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> // Fornece funções e estruturas para comunicação de sockets
#include <sys/stat.h> // Para a função stat
#include <dirent.h> // Para manipulação de diretorios

#define PORT 8080 // Porta que o servidor irá escutar
#define BUFFER_SIZE 3000 // Tamanho do buffer para leitura das requisições

// Função para listar o conteúdo de um diretório
void listar_diretorio(int socket, const char *caminho_dir) {
    char body[BUFFER_SIZE * 2] = "<html><head><title>Listagem de Diretório</title></head><body>";
    strcat(body, "<h1>Conteúdo de ");
    strcat(body, caminho_dir);
    strcat(body, "</h1><ul>");

    DIR *d = opendir(caminho_dir);
    if (d) {
        struct dirent *diretorio;
        while ((diretorio = readdir(d)) != NULL) {
            // Ignora "." e ".."
            if (strcmp(diretorio->d_name, ".") == 0 || strcmp(diretorio->d_name, "..") == 0)
                continue;

            char item[1024];
            // Cria link relativo ao diretório atual
            snprintf(item, sizeof(item),
                     "<li><a href=\"%s\">%s</a></li>",
                     diretorio->d_name, diretorio->d_name);
            strcat(body, item);
        }
        closedir(d);
    } else {
        strcat(body, "<li>Erro ao abrir diretório</li>");
    }

    strcat(body, "</ul></body></html>");

    // Envia cabeçalho HTTP
    char cabecalho[256];
    snprintf(cabecalho, sizeof(cabecalho),
             "HTTP/1.1 200 OK\r\nContent-Length: %ld\r\nContent-Type: text/html\r\n\r\n",
             strlen(body));

    (void) write(socket, cabecalho, strlen(cabecalho));
    (void) write(socket, body, strlen(body));
}

// Função para determinar o tipo do arquivo com base na extensão
const char *detectar_tipo_mime(const char *caminho) {
    if(strstr(caminho, ".html")) return "text/html";
    if(strstr(caminho, ".jpg") || strstr(caminho, ".jpeg")) return "image/jpeg";
    if(strstr(caminho, ".png")) return "image/png";
    if(strstr(caminho, ".gif")) return "image/gif";
    if(strstr(caminho, ".css")) return "text/css";
    if(strstr(caminho, ".js")) return "application/javascript";
    if(strstr(caminho, ".pdf")) return "application/pdf";
    if(strstr(caminho, ".txt")) return "text/plain";
    return "application/octet-stream";
}

void enviar_arquivo(int socket, const char *caminho){
    // Função para enviar o conteúdo de um arquivo
    FILE *arquivo = fopen(caminho, "rb");
    if (arquivo == NULL) {
        // Arquivo não encontrado, erro 404
        const char *erro =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n\r\n"
            "<html><body><h1>404 - Arquivo não encontrado</h1></body></html>";
        write(socket, erro, strlen(erro));
        return;
    }

    // Descobre o tamanho do arquivo
    fseek(arquivo, 0, SEEK_END);
    long tamanho = ftell(arquivo);
    rewind(arquivo);

    // Cabeçalho HTTP de resposta
    char cabecalho[256];
    const char *tipo = detectar_tipo_mime(caminho);
    snprintf(cabecalho, sizeof(cabecalho), "HTTP/1.1 200 OK\r\n" "Content-Length: %ld\r\n" "Content-Type: %s\r\n\r\n", tamanho, tipo);
    write(socket, cabecalho, strlen(cabecalho));

    // Envia o conteúdo do arquivo
    char buffer[BUFFER_SIZE];
    size_t bytes_lidos;
    while((bytes_lidos = fread(buffer, 1, sizeof(buffer), arquivo)) > 0){
        write(socket, buffer, bytes_lidos);
    }

    fclose(arquivo);
}

// Função para montar o caminho completo do arquivo
void montar_caminho(char *destino, const char *base_diretorio, const char *caminho) {
    if (strcmp(caminho, "/") == 0)
        snprintf(destino, 1024, "%s/index.html", base_diretorio);
    else
        snprintf(destino, 1024, "%s%s", base_diretorio, caminho);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Use: %s <diretorio_base>\n", argv[0]);
        return 0;
    }

    const char *base_diretorio = argv[1];
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Criando o socket: a função socket cria um socket com os parâmetros:
    // AF_INET: especifica o uso do protocolo IPv4
    // SOCK_STREAM: define o uso do protocolo TCP
    // O valor de retorno é um descritor de arquivo que representa o socket. Se retornar 0, houve um erro
    if((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0){
        printf("Erro ao criar o socket\n");
        return 0;
    }

    int opt = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0){
        perror("setsockopt(SO_REUSEADOR) failed");
        return 0;
    }

    // Configurando o endereço do servidor
    // sin_family: define a família do protocolo como IPv4
    // sin_addr.s_addr: configura o endereço IP. INADDR_ANY permite que o 
    // servidor execute em todos os IPs disponíveis
    // sin_port: define a porta, convertida para o formato de rede com htons
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Ligando o socket ao endereço e porta
    if(bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0){
        printf("Erro ao associar o socket\n");
        close(server_fd); // Se falhar, close é usado para liberar recursos
        return 0;
    }

    // Colocando o socket em modo de escuta
    // O "3" define a capacidade da fila de conexões pendentes
    if (listen(server_fd, 3) < 0) {
        printf("Erro ao escutar\n");
        close(server_fd);
        return 0;
    }

    printf("Servidor HTTP iniciado na porta %d...\n", PORT);

    // Loop para aceitar conexões
    while(1){
        printf("\nAguardando conexao...\n");
        // accept bloqueia o programa até que uma conexão seja aceita.
        // Retorna um novo descritor para o socket conectado
        if((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0){
            printf("Erro ao aceitar conexão\n");
            continue;
        }

        // Lendo a solicitação do cliente (opcional, dependendo do uso)
        read(new_socket, buffer, BUFFER_SIZE);
        printf("Requisição recebida:\n%s\n", buffer);

        // Extraindo método, caminho e versão
        char method[10], path[1024], version[20];
        if(sscanf(buffer, "%s %s %s", method, path, version) != 3){
            printf("Nao foi possivel interpretar a requisicao.\n");
            close(new_socket);
            continue;
        }

        // Monta o caminho completo do recurso pedido
        char caminho_completo[1024];
        snprintf(caminho_completo, sizeof(caminho_completo), "%s%s", base_diretorio, path);
        printf("Recurso solicitado: %s\n", caminho_completo);

        struct stat path_stat;
        if(stat(caminho_completo, &path_stat) == -1){
            const char *erro =
                "HTTP/1.1 404 Not Found\r\n"
                "Content-Type: text/html\r\n\r\n"
                "<html><body><h1>404 - Arquivo não encontrado</h1></body></html>";
            (void) write(new_socket, erro, strlen(erro));
        }
        else if(S_ISDIR(path_stat.st_mode)){
            // Se for diretório, tenta o index.html
            char caminho_index[2048];
            snprintf(caminho_index, sizeof(caminho_index), "%s/index.html", caminho_completo);

            if(access(caminho_index, F_OK) == 0){
                enviar_arquivo(new_socket, caminho_index);
            } else {
                listar_diretorio(new_socket, caminho_completo);
            }
        }
        else if(S_ISREG(path_stat.st_mode)){
            enviar_arquivo(new_socket, caminho_completo);
        }
        // Fechando o socket do cliente
        close(new_socket);
    }

    // Fechando o socket do servidor
    close(server_fd);

    return 0;
}