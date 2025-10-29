# Projeto Protocolo HTTP em C

Esse projeto implementa um **servidor HTTP** e um **cliente HTTP** em C.
O servidor serve arquivos de um diretório, enquanto o cliente realiza requisições HTTP e salva os arquivos localmente.

## Visão Geral do Projeto

### Servidor HTTP

O servidor é capaz de receber requisições HTTP e servir arquivos de um diretório local especificado na inicialização.

**Funcionalidades**
* Serve arquivos como HTML, imagens, pdfs, etc.
* Se um diretório for requisitado, ele busca e retorna o arquivo 'index.html' contido nele.
* Caso o arquivo 'index.html' não esteja no diretório, o servidor retorna uma página com a listagem de todos os arquivos e pastas daquele diretório.
* Opera na porta '8080' por padrão.

### Cliente HTTP

O cliente é uma ferramenta de linha de comando que faz uma requisição GET para uma URL.

**Funcionalidades:**
* Baixa um arquivo a partir de uma URL fornecida como argumento.
* Salva o arquivo no diretório onde o comando foi executado.
* Extrai o nome do arquivo a partir da URL.

## Estrutura do Projeto

O projeto está organizado da seguinte forma para separar claramente as responsabilidades do cliente e do servidor.

```
.
├── servidor
│   ├── servidor.c      
│   └── Makefile      
├── cliente
│   ├── cliente.c  
│   └── Makefile
├── site 
│   ├── index.html
│   ├── cardapio_ru.pdf
│   └── contagem_regressiva.jpeg
├── LICENSE  
└── README.md 
```

* `cliente/` → Código-fonte e Makefile do cliente HTTP  
* `servidor/` → Código-fonte e Makefile do servidor HTTP  
* `site/` → Diretório de teste com arquivos servidos pelo servidor 

## Compilação

### Cliente

```bash
cd cliente
make
```

### Servidor

```bash
cd servidor
make
```

### Limpeza (remover binários)

```bash
make clean
```

## Execução

### Rodando o Servidor

Você precisa informar o diretório que será servido:

```bash
./servidor <diretorio_base>
```

### Rodando o Cliente
Para baixar um arquivo usando o cliente:

```bash
./cliente http://<host>:<porta>/<arquivo>
```

## Licença

Este projeto está sob a licença **MIT**. Veja o arquivo `LICENSE` para mais informações.
