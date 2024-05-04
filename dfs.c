/*
	C socket server example, handles multiple clients using threads
*/

#include<stdio.h>
#include<string.h>	//strlen
#include<stdlib.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include<pthread.h> //for threading , link with lpthread
#include <dirent.h> //For directories
#include <errno.h>
#include <sys/stat.h> //mkdir()

//the thread function
void *connection_handler(void *);
void get_command(int sock);
void put_command(int sock);
void list_command(int sock);
int number;
char filename[100];

int main(int argc , char *argv[])
{
	int socket_desc , client_sock , c , *new_sock;
	struct sockaddr_in server , client;
	int portno; /* port to listen on */


	/* 
   * check command line arguments 
   */
  	if (argc != 3) {
    	fprintf(stderr, "usage: %s <port> <Timeout> \n", argv[0]);
    	exit(1);
  	}
  	portno = atoi(argv[1]);
	number = atoi(argv[1]) - 10000;
	sprintf(filename, "dfs%d/", number);

	
	//Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");

	
	
	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
    //server.sin_addr.s_addr = inet_addr("10.224.76.120");
	server.sin_port = htons( portno );
	
	//Bind
	if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
	{
		//print the error message
		perror("bind failed. Error");
		return 1;
	}
	puts("bind done");
	
	//Listen
	listen(socket_desc , 3);
	
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	
	
	//Accept and incoming connection
	puts("Waiting for incoming connections...");
	c = sizeof(struct sockaddr_in);
	while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
	{
		//puts("Connection accepted");
		
		pthread_t sniffer_thread;
		new_sock = malloc(1);
		*new_sock = client_sock;
		
		if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
		{
			perror("could not create thread");
			return 1;
		}
		
		//Now join the thread , so that we dont terminate before the thread
		//pthread_join( sniffer_thread , NULL);
        //Send some messages to the client
	    
	
	    // char *message = "Greetings! I am your connection handler\nNow type something and i shall repeat what you type \n";
	    // send(client_sock , message , strlen(message),0);
		//puts("Handler assigned");
	}
	
	if (client_sock < 0)
	{
		perror("accept failed");
		return 1;
	}
	
	return 0;
}

/*
 * This will handle connection for each client
 * */
void *connection_handler(void *socket_desc)
{
    int sock = *(int*)socket_desc;
    int read_size;
    char client_message[20000];
	
    

    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
	{
		if (send(sock, "Ack", sizeof("Ack"), 0) == -1) {
            perror("Error sending data");
            exit(EXIT_FAILURE);
        }
        if (strcasecmp(client_message, "put") ==0){
        	put_command(sock);
    		}
    	else if (strcasecmp(client_message, "get") ==0){
        	get_command(sock);
    	}
    	else if (strcasecmp(client_message, "list") ==0){
        	list_command(sock);
    	}
    	else{
        	printf("INCORRECT COMMAND");
        	exit(1);
    	}
    }

        
}	

void put_command(int sock){
	int read_size_1,read_size_2,read_size_3;
	char second_message[2000];
    char third_message[2000];
	char folder_name[50]; // Adjust size as needed
	char second_file_path[100]; // Adjust size as needed
	char third_file_path[100]; // Adjust size as needed
	int first_chunk, second_chunk;
	long long first_message;
	long long result;
	// Receive the first message (assuming it's an integer)
        if ((read_size_1 = recv(sock, &first_message, sizeof(long long), 0)) < 0) {
            perror("recv");
        }
		// //ACK
		if (send(sock, "Ack", sizeof("Ack"), 0) == -1) {
            perror("Error sending data");
            exit(EXIT_FAILURE);
        }
        // Receive the second message
        if ((read_size_2 = recv(sock, second_message, sizeof(second_message), 0)) < 0) {
            perror("recv");
        }
		for (volatile int i = 0; i < 99999; i++){ if (i%3==0){ i++;}}
        // Receive the third message
        if ((read_size_3 = recv(sock, third_message, sizeof(third_message), 0)) < 0) {
            perror("recv");
        }

        // Print received messages
        printf("First message (int): %lld\n", first_message);
        printf("Second message (string): %s\n", second_message);
        printf("Third message (string): %s\n", third_message);
		result = first_message%4; if (result < 0){result = result * -1;}
		switch(result){
        case 0:
            first_chunk = number%4;
            second_chunk = (number+1)%4;
            break;
        case 1:
            first_chunk = (number-1)%4;
            second_chunk = (number)%4;
            break;
        case 2:
            first_chunk = (number+2)%4;
            second_chunk = (number+3)%4;
            break;
        case 3:
            first_chunk = (number+1)%4;
            second_chunk = (number+2)%4;
            break;
        default:
            printf("ERROR WRONG HASH OF FILENAME");
            break;
    }
    printf("NUMBER: %d My Chunks are %d and %d",number, first_chunk, second_chunk);
	fflush(stdout);
	snprintf(folder_name, sizeof(folder_name), "dfs%d/%lld",number, first_message);
    if (mkdir(folder_name, 0777) != 0) {
        perror("mkdir");
    }

	snprintf(second_file_path, sizeof(second_file_path), "%s/file1.bin", folder_name);
    FILE *file1 = fopen(second_file_path, "wb");
    if (file1 == NULL) {
        perror("fopen");
        return;
    }
    if (fwrite(second_message, 1, read_size_2, file1) != read_size_2) {
    perror("fwrite");
    fclose(file1);
}
    fclose(file1);

	snprintf(third_file_path, sizeof(third_file_path), "%s/file2.bin", folder_name);
    FILE *file2 = fopen(third_file_path, "wb");
    if (file2 == NULL) {
        perror("fopen");
        return;
    }
    if (fwrite(third_message, 1, read_size_3, file2) != read_size_3) {
    perror("fwrite");
    fclose(file1);
	}
    fclose(file2);
}

void get_command(int sock){
	int read_size_1;
	int first_chunk, second_chunk;
	long long first_message;
	long long result;
	char hash[21];
	char buffer[10000];
	// Receive the first message (assuming it's an integer)
        if ((read_size_1 = recv(sock, &first_message, sizeof(long long), 0)) < 0) {
            perror("recv");
        }
		printf("First message (int): %lld\n", first_message);
	result = first_message%4; if (result < 0){result = result * -1;}
		switch(result){
        case 0:
            first_chunk = number%4;
            second_chunk = (number+1)%4;
            break;
        case 1:
            first_chunk = (number-1)%4;
            second_chunk = (number)%4;
            break;
        case 2:
            first_chunk = (number+2)%4;
            second_chunk = (number+3)%4;
            break;
        case 3:
            first_chunk = (number+1)%4;
            second_chunk = (number+2)%4;
            break;
        default:
            printf("ERROR WRONG HASH OF FILENAME");
            break;
    }
    printf("NUMBER: %d My Chunks are %d and %d",number, first_chunk, second_chunk);
	fflush(stdout);
	//First File
	sprintf(filename, "dfs%d/", number);
	sprintf(hash, "%lld", first_message);
	strcat(filename,hash);
	strcat(filename,"/");
	strcat(filename,"file1.bin");
	printf("\n\n%s\n\n",filename);
	// Open the file
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("File opening failed");
        exit(EXIT_FAILURE);
    }
	size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
		printf("First File Sent%ld",bytes_read);
        if (send(sock, buffer, bytes_read, 0) == -1) {
            perror("Error sending data");
            exit(EXIT_FAILURE);
        }
    }
	fclose(file);
	printf("First File Sent%ld",bytes_read);
	// Receive ACK
        if ((bytes_read = recv(sock, buffer, sizeof(buffer), 0)) < 0) {
            perror("recv");
        }
	//Second File
	sprintf(filename, "dfs%d/", number);
	sprintf(hash, "%lld", first_message);
	strcat(filename,hash);
	strcat(filename,"/");
	strcat(filename,"file2.bin");
	printf("\n\n%s\n\n",filename);
	// Open the file
    file = fopen(filename, "rb");
    if (!file) {
        perror("File opening failed");
        exit(EXIT_FAILURE);
    }
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), file)) > 0) {
        if (send(sock, buffer, bytes_read, 0) == -1) {
            perror("Error sending data");
            exit(EXIT_FAILURE);
        }
    }
	printf("Done");
	fflush(stdout);
	close(sock);
}

void list_command(int sock){

}
