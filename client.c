#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<string.h>
#include<netinet/in.h>
#include<errno.h>
#include<netdb.h>
#include<fcntl.h>
#include<time.h>

int sockfd = 0; //store file desc for socket
int noteFlag; //0 - without note -> 1 - with
char* iban; //will store the iban
int money = 0; //will store amount of money for withdraw
int moneyInTheBank = 0; //buffer for money on stock
int customersNumber = 0; //total number of customers
int language = 0;//0-english, 1-bulgarian
int correctIBAN = 0;//0-incorrect iban+pin, 1-correct iban+pin

struct CustomerInfo {
	char i[12];//will store client`s info
} info[50000];

char* getCurrentTime() {
	char* buff;
	time_t rawtime;
	struct tm* time_info;
	time(&rawtime);
	time_info = localtime(&rawtime);
	buff = malloc(200);
	sprintf(buff, "%d-%d-%d %d:%d:%d", time_info->tm_mday, time_info->tm_mon+1, time_info->tm_year+1900, time_info->tm_hour, time_info->tm_min, time_info->tm_sec);
	return buff;
}

int convertToInt(char* num) {
	char *temp = num;
	int size = 0;
	int j = 0, number = 0, multiplier = 1;

	while(*temp != '\0') { //get the size of the "number"
		size++;
		temp++;
	}

	for(j = size - 1; j >= 0; j--) { //size - 1 (\0)
		if(num[j] >= '0' && num[j] <= '9') {
			number += (multiplier * (num[j] - '0'));
			multiplier *= 10; //ones, tens, hundreds, thousands...
		}
	}
	return number;
}

void convertToString(int num, int out) {
	int j, k, divider = 10;
	char number[4];
	for(j = 0; j < 4; j++) {
		if(num % divider >= 0 || num % divider <= 9) {
			number[j] = (num % divider) + '0';
			num /= 10;
		}
	}
	for(j = 3, k = 8; j >= 0, k < 12; j--, k++) {
		info[out].i[k] = number[j];
	}
	//update the file
	int wr = open("/home/jordan/Documents/ex.txt", O_WRONLY, 0666);
	for(j = 0; j < customersNumber; j++) {
		write(wr, info[j].i, 12);
		write(wr, "\n", 1);
	}
}

void calculateWithdraw(int num) {
	int j, k = 0;
	char* numb = malloc(5);//will store 4 "digits"
	for(j = 8; j < 12; j++) {
		numb[k++] = info[num].i[j];
	}
	numb[k] = '\0';
	moneyInTheBank = convertToInt(numb);
	fflush(stdin);
	if((money > moneyInTheBank) && (language == 0)) {
		printf("You don`t have enough money in this account! Transaction is terminated!\n");
		exit(0);
	} else if ((money > moneyInTheBank) && (language == 1)) {
		printf("Недостатъчна наличност! Операцията е блокирана!\n");
		exit(0);
	} else {
		moneyInTheBank -= money;
		convertToString(moneyInTheBank, num);
	}
}

void getIBAN() {
	iban = malloc(8); //IBAN = BankAccount(4 chars) + PIN(4 chars) = 8 chars(bytes)
	if(language == 0) iban = getpass("Enter your IBAN: "); //getpass wont show symbols on stdout
	if(language == 1) iban = getpass("Въведете своя IBAN:");
}

void treasuryNote() {
	char ch[10];
	sleep(1);
	bzero(ch, 10);
	fflush(stdin);
	scanf("%s", &ch);
	switch(ch[0]) {
	case 'Y':
	case 'y': noteFlag = 1;
		  break;
	case 'N':
	case 'n': noteFlag = 0;
		  break;
	default: if(language == 0) printf("Wrong input!");
		 if(language == 1) printf("Грешка при въвеждане\n");
		 break;
	}
}

void transaction(int sockfd) {
	char buff[1024];
	int in = 0, out = 0;
	fflush(stdin);
	printf("Enter language: ");
	fgets(buff, 1024, stdin);
	switch(buff[0]) {
	case 'E':
	case 'e':
  	  	  bzero(info, sizeof(struct CustomerInfo));
		  while(read(sockfd, &info[in], sizeof(struct CustomerInfo)) > 0) {
			in++;
		  }
		  customersNumber = in;
		  printf("How much money do you want to withdraw? ");
		  fgets(buff, 1024, stdin);
		  money = convertToInt(buff);
		  fflush(stdin);
		  printf("Do you want treasury note? (Y/N) ");
		  treasuryNote();
		  fflush(stdin);
		  getIBAN();
		  for(out = 0; out < in; out++) {
		  	if(strncmp(info[out].i, iban, 8) == 0) {
				correctIBAN = 1;
				calculateWithdraw(out);
		  	}
		  }
		  free(iban);
		  if(noteFlag && correctIBAN) {
			printf("We got your PIN! Congratulations! \u263A\n");
			printf("Withdrawed: $%d\n", money);
			printf("Money in account: $%d\n", moneyInTheBank);
			printf("Date and time of transaction: %s\n", getCurrentTime());
			printf("Goodbye!\n");
		  } else if(!correctIBAN){
			printf("Incorrect PIN!\nTransaction is terminated\n");
		  }
		  break;
	case 'B':
	case 'b': language = 1;
		  bzero(info, sizeof(struct CustomerInfo));
		  while(read(sockfd, &info[in], sizeof(struct CustomerInfo)) > 0) {
			in++;
		  }
		  customersNumber = in;
		  printf("Сума, която искате да изтеглите? ");
		  fgets(buff, 1024, stdin);
		  money = convertToInt(buff);
		  fflush(stdin);
		  printf("Желаете ли разписка? (Y/N) ");
		  treasuryNote();
		  fflush(stdin);
		  getIBAN();
		  for(out = 0; out < in; out++) {
			if(strncmp(info[out].i, iban, 8) == 0) {
				correctIBAN = 1;
				calculateWithdraw(out);
			}
		  }
		  free(iban);
		  if(noteFlag && correctIBAN) {
		  	printf("Записахме Вашия ПИН! Честито! \u263A\n");
			printf("Изтеглихте %d лева\n", money);
			printf("Наличност след транзакцията: %d лева\n", moneyInTheBank);
			printf("Дата и час на транзакцията: %s\n", getCurrentTime());
			printf("Довиждане!\n");
		  } else if(!correctIBAN){
			printf("Грешен PIN! Операцията е блокирана!\n");
		  }
		  break;
	default: printf("Wrong language!");
		 break;
	}
}

int main(int argc, char *argv[]) {

	int n = 0, portno;
	char buff[1024];
	struct sockaddr_in serv_addr;
	struct hostent *server;

	if(argc < 3) {
		fprintf(stderr, "usage: %s hostname portn\n", argv[0]);
		exit(0);
	}

	portno = atoi(argv[2]);

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		printf("Error creating socket\n");
		return 1;
	}
	server = gethostbyname(argv[1]);
	if(server == NULL) {
		perror("Couldn`t find the server!\n");
		exit(1);
	}
	bzero(buff, 1024);
	bzero((char *) &serv_addr, sizeof(serv_addr));


	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);


	if(connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
		printf("Error in connecting\n");
		exit(1);
	}

	transaction(sockfd);

	close(sockfd);
	return 0;
}
