#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

char header[4096];
void * handle_client(void *p_socket);
//create the HTTP header to send to the client
void httpHeader(int *client_socket,char *mime, int tamanho){
	char buffer[100];
	char http[79] = "HTTP/1.0 200 OK\r\nServer: FileServer\r\nDate: Sat, 02 Jul 2022 16:22:12 GMT\r\n";
	char last[48] = "Last-Modified: Fri, 01 Jul 2022 14:19:06 GMT\r\n\n";
	
  sprintf(buffer, "Content-Type: %s\r\n", mime);
	send(*client_socket, http, strlen(http), 0);
	send(*client_socket, buffer, strlen(buffer), 0);

	sprintf(buffer, "Content-Length: %d\r\n", tamanho);
	send(*client_socket, buffer, strlen(buffer), 0);
	send(*client_socket, last, strlen(last), 0);

	return NULL;
}
//separete the message into method, file name and version of the HTTP protocol
char *request(char *msg) {
	char *metodo = malloc(4);
	char *recurso = malloc(1000);
	char *protocolo = malloc(20);

	metodo = strtok(msg, " ");
	recurso = strtok(NULL, " ");
	protocolo = strtok(NULL, "\r");
    
	return recurso+1;//RETURNS THE NAME OF THE FILE WITHOUT "/"
}
//get the MIME type of the file to send in the header
char *mime_type(char *extensao){
	char *mime = malloc(19);
	if (!extensao) return NULL;
	if (strcmp(extensao, ".html") == 0 || strcmp(extensao, ".htm") == 0) sprintf(mime,"text/html");
	if (strcmp(extensao, ".c") == 0 || strcmp(extensao, ".txt") == 0) sprintf(mime,"text/plain");
	if (strcmp(extensao, ".jpg") == 0 || strcmp(extensao, ".jpeg") == 0) sprintf(mime,"image/jpeg");
	if (strcmp(extensao, ".gif") == 0) sprintf(mime,"image/gif");
	if (strcmp(extensao, ".png") == 0) sprintf(mime,"image/png");
	if (strcmp(extensao, ".avi") == 0) sprintf(mime,"video/x-msvideo");
	if (strcmp(extensao, ".mpeg") == 0 || strcmp(extensao, ".mpg") == 0) sprintf(mime,"video/mpeg");
	if (strcmp(extensao, ".mp3") == 0) sprintf(mime,"audio/mpeg");
	if (strcmp(extensao, ".au") == 0) sprintf(mime,"audio/basic");
	if (strcmp(extensao, ".pdf") == 0) sprintf(mime,"application/pdf");
	if (strcmp(extensao, ".zip") == 0) sprintf(mime,"application/zip");
	if (strcmp(extensao, ".gz") == 0) sprintf(mime,"application/gzip");
	if (strcmp(extensao, ".doc") == 0) sprintf(mime,"application/msword");
	if (strcmp(extensao, ".css") == 0) sprintf(mime,"text/css");
	if (strcmp(extensao, ".xml") == 0) sprintf(mime,"application/xml");
	return mime;
}
//when the file is not found, send a HTML with the message not found
void notFound(int *client_socket){
	char http[164] = "HTTP/1.0 404\r\nServer: FileServer\r\nDate: Sat, 02 Jul 2022 16:22:12 GMT\r\nContent-Type: text/html\r\nContent-Length: 87\r\nLast-Modified: Fri, 01 Jul 2022 14:19:06 GMT\r\n\n";
	char html[87] = "<html>\n<head>\n<title>Erro 404</title>\n</head>\n\n<body>\n<h1>Erro 404</h1>\n</body></html>";
	send(*client_socket, http, strlen(http), 0);
	send(*client_socket, html, strlen(html), 0);
	return NULL;
}


//function to get the file
void buscarArquivo(char *nome_arquivo,int *client_socket){

	char *extensao = strrchr(nome_arquivo, '.');
	char *ext = mime_type(extensao);

	FILE *file;
	char *buffer;
	int tamanho;

	//open the file in binary read mode
	file = fopen(nome_arquivo, "rb");
	if(!file){
		notFound(client_socket);
		return;
	}


	//get the size of the file
	fseek(file, 0, SEEK_END);
	tamanho=ftell(file);
	fseek(file, 0, SEEK_SET);

	httpHeader(client_socket,ext,tamanho);
	buffer=(char *)malloc(tamanho+1);
	if (!buffer)
	{
		fprintf(stderr, "Memory error!");
		fclose(file);
		return;
	}
	fread(buffer, tamanho, 1, file);
	fclose(file);
	send(*client_socket, buffer, tamanho, 0);
	return NULL;

}
//function to use thread
void *handle_client(void *p_socket){
	int *client_socket = *((int*)p_socket);
	free(p_socket);
	char msg[1024];
	int i_msg;
	while(1){
	bzero(msg, 1024);
		i_msg = read(client_socket, msg, 1024);
		if(msg < 0)
			perror("Erro de recebimento");
		printf("\n\nMsg do client: %s\n",msg);
		int i = strncmp("GET / ", msg, 6);
		if(i <= 0){
			char *resposta = request(&msg);
			buscarArquivo(resposta,&client_socket);
			close(client_socket);
			bzero(msg, 1024);
			break;
		}else{
			notFound(&client_socket);
			close(client_socket);
		}
	}
}


int main(){ 
	int s;
	s = socket(AF_INET, SOCK_STREAM, 0);
	if(s < 0){
		perror("Erro ao abrir o socket");
	}
	struct sockaddr_in server_address, client_address;
	bzero((char *) &server_address, sizeof(server_address));
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(80);
	server_address.sin_addr.s_addr = INADDR_ANY;

	if(bind(s,(struct sockaddr*) &server_address, sizeof(server_address)) < 0)
		perror("Erro ao bindar");

	listen(s, 1024);
	socklen_t soc_cli_size;
	soc_cli_size = sizeof(client_address);//tamanho do client 
	int client_socket;
  
	while (1)
	{
		client_socket = accept(s, (struct sockaddr *) &client_address, &soc_cli_size);

		//print out the client information *_*
		char client_ip[INET_ADDRSTRLEN];//char com o tamanho INET ??PESQUISAR MAIS 
		inet_ntop(AF_INET, &client_address.sin_addr, client_ip, sizeof(client_ip));//converte o endereco em forma de char para printar
		printf("Client ip %s",client_ip);
		printf(" on port %d", ntohs(client_address.sin_port)); //print out the port of the client connected

		int *pclient = malloc(sizeof(int));
		*pclient = client_socket;
		pthread_t t;
		pthread_create(&t,NULL,handle_client,pclient);
	}
	close(s);

	return 0;
}
