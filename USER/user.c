#include "aux.h"

#define DEFAULT_PORT		1337
#define DEFAULT_HOSTNAME	"localhost"
#define MAX_INPUT_MSG_LENGTH	50 //TODO Change?
int initClient(char* ip, int port) { //initialize connection, returns -1 on errors, otherwise socket
	int sockfd;
	struct sockaddr_in serv_addr;
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) //Error creating socket
	{
		return -1;
	}

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port); // change?
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); //change?
	bzero(&(serv_addr.sin_zero), 8);
	//printf("Port is %d",port);

	size_t addr_size = sizeof(serv_addr);
	if (connect(sockfd, (struct sockaddr *) &serv_addr, addr_size) < 0) { //Error connecting
		perror("Error starting connection \n");
		error(1);
	}

	return sockfd;
}

void parseInputMsg(char* msg, int sockfd) { //Parse input msg and call appropriate function

	const char s[2] = " ";
	char *token;
	token = strtok(msg, s); //Get first word from input

	if (strcmp(token, "list_of_files") == 0) {
		//list_of_files(sockfd);
	} else if (strcmp(token, "delete_file") == 0) {
		//	token = strtok(NULL, s);
		//delete_file(sockfd, token);
	} else if (strcmp(token, "add_file") == 0) {
		//char* path = token = strtok(NULL, s);
		//	char* filename = token = strtok(NULL, s);
		//add_file(sockfd, path, filename);
	} else if (strcmp(token, "get_file") == 0) {
		//char* filename = token = strtok(NULL, s);
		//	char* path = token = strtok(NULL, s);
		//get_file(sockfd, filename, path);
	} else if (strcmp(token, "quit") == 0) {
		//quit(sockfd);
	}

	else {
		printf("Error: unexpected message\n");
	}

}

int main(int argc, char *argv[]) {
	if ((argc != 3) && (argc != 1)) {
		printf("should receive 2 or 0 cmd args. Received %d args", argc);
	}

	char* hostname = DEFAULT_HOSTNAME;
	int port = DEFAULT_PORT;
	char username[MAX_USERNAME_LENGTH] = "";
	char password[MAX_PASSWORD_LENGTH] = "";

	if (argc == 3) {
		hostname = argv[1];
		port = atoi(argv[2]);
	}
	int clientSocket = initClient(hostname, port);
	if (clientSocket == -1) {
		perror("Error starting connection \n");
	}

	//recieve hello msg from server
	Message responseMsg = receiveMessage(clientSocket);
	if (responseMsg.msg_type == helloMSG) {
		printf("Welcome! Please log in. /n ");
	}
	if (responseMsg.msg_type != helloMSG) {
		printf("Error recieveing hello msg /n ");
	}

	//Get login input from user and send login msg to server
	printf("User: /n");
	scanf("%s", username);
	printf("Password: /n");
	scanf("%s", password);
	int status;
	Message msg = createMessagefromTwoStrings(loginMSG, username, password);
	//Send request to server
	status = sendMessage(clientSocket, msg);
	if (status == 0) {
		perror("Error sending login msg");
	}

	//Recieve login result from server
	responseMsg = receiveMessage(clientSocket);
	if (responseMsg.msg_type == successMSG) {
		responseMsg = receiveMessage(clientSocket); //receive status msg
		printf("Hi Bob, you have 8 files stored /n "); //TODO edit
	}
	if (responseMsg.msg_type == failureMSG) {
		printf("Authentication failed/n ");
	}

	//Get continuous input from user and call appropriate function
	char input[MAX_INPUT_MSG_LENGTH];
	fgets(input, MAX_INPUT_MSG_LENGTH, stdin);
	while (strcmp(input, "quit\n") != 0) { //Keep getting input until "quit" is received
		parseInputMsg(input, clientSocket);
		fgets(input, MAX_INPUT_MSG_LENGTH, stdin);
	}

	quit(clientSocket);
}

void list_of_files(int clientSocket) {
	int status;
	Message msg = createMessagefromString(list_of_filesMSG, NULL); //TODO check if I should replace NULL with an empty string

	//Send request to server
	status = sendMessage(clientSocket, msg);
	if (status == 0) {
		perror("Error sending list_of_files msg");
	}

	//Recieve response
	Message responseMsg = receiveMessage(clientSocket);
	printf(responseMsg.value);

}

void add_file(int clientSocket, char* path_to_file, char* newFileName) {
	int status;
	FILE *fp;

	//Open the file and read its content into buffer
	fp = fopen(path_to_file, "r");
	char* buffer = malloc(sizeof(char) * BUFFER_SIZE);
	fread(buffer, sizeof(char), BUFFER_SIZE, fp);
	fclose(fp);

	Message msg = createMessagefromTwoStrings(transfer_fileMSG, newFileName,
			path_to_file);

	//Send request to server
	status = sendMessage(clientSocket, msg);
	if (status == 0) {
		perror("Error sending transfer_fileMSG msg");
	}

	//Recieve response
	Message responseMsg = receiveMessage(clientSocket);
	printf(responseMsg.value);

	//Check result of recieved msg from server
	if (responseMsg.msg_type == successMSG) {
		printf("File added");
	} else {
		printf("Error adding file");
	}

}


void delete_file(int clientSocket, char* filename) {
int status;
//Send delete filename msg to server
Message msg = createMessagefromString(delete_fileMSG, filename);
status = sendMessage(clientSocket, msg);
if (status == 0) {
	perror("Error sending delete_file msg");
}

//Recieve response
Message responseMsg = receiveMessage(clientSocket);

//Check result of recieved msg from server
if (responseMsg.msg_type == successMSG) {
	printf("File removed");
} else {
	printf("No such file exists!");
}
}

void get_file(int clientSocket, char* file_name, char* path_to_save) {

int status;
//Send get file request to server
Message msg = createMessagefromString(get_fileMSG, file_name);
status = sendMessage(clientSocket, msg);
if (status == 0) {
	perror("Error sending get_fileMSG msg");
}

//Recieve response from server
Message responseMsg = receiveMessage(clientSocket);

//Check result of recieved msg from server
if (responseMsg.msg_type == failureMSG) {
	printf("Error getting file");
} else {
	//Get file content from server and save it
	char* buffer = malloc(sizeof(char) * BUFFER_SIZE);
	buffer = responseMsg.value; //Get file content into buffer
	FILE *fp;
	fp = fopen(path_to_save, "w");
	fwrite(buffer, sizeof(char), BUFFER_SIZE, fp);
	fclose(fp);

	printf("File saved");
}

}

void quit(int clientSocket) {
int status;
//Send quit msg to server
Message msg = createMessagefromString(quitMSG, "");
status = sendMessage(clientSocket, msg);
if (status == 0) {
	perror("Error sending quitMSG msg");
}
if (close(socket) == -1) {
	printf("close failed \n");
} else {
	printf("close succeeded \n");
}
}

