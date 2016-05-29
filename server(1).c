#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include<unistd.h>
#include<netinet/in.h>
#include<fcntl.h>
#include<pthread.h>

void checkClientsInfo();
void* transaction(void*);

pthread_t t;
int connection = 0, out = 0, in = 0;

struct CustomerInfo {
	char i[12];
} info[50000];

void checkClientsInfo() {
	int fd = 0;//will store the descriptor of "DB"
	char buff;
	bzero(info, sizeof(struct CustomerInfo));

	fd = open("ex.txt", O_RDONLY);
	if(fd < 0) {
		perror("Couldn`t read the file\n");
		exit(1);
	}

	in = 0;
	while(read(fd, &info[in], 12) > 0) {
		read(fd, &buff, sizeof(char)); //newline char
		in++;
	}
}

void* transaction(void* kashlica) {
	for(out = 0; out < in; out++) {
		write(connection, &info[out], 12);
	}
}

int main(int argc, char* argv[]) {

	int listenfd = 0, c = 0, fd = 0;
	int portno = 0;
	struct sockaddr_in serv_addr, client;

	if(argc < 2) {
		fprintf(stderr, "Error, no port provided\n");
		exit(1);
	}

	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	memset(&serv_addr, '0', sizeof(serv_addr));

	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(portno);

	bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

	if (listen(listenfd, 50000) == -1) {
		perror("An error occurred!\n");
		exit(1);
	}

	while(1) {
		int thread;
		connection = accept(listenfd, (struct sockaddr*)&client, (socklen_t*)&c);
		if(connection < 0) {
			perror("Couldn`t accept client!\n");
			exit(1);
		}
		checkClientsInfo();
		printf("Client accepted!\n");
		thread = pthread_create(&t, NULL, transaction, NULL);
		pthread_join(t, NULL);
		close(connection);
	}
	close(listenfd);
	return 0;
}
