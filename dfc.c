
#include<stdio.h>
#include<string.h>	//strlen
#include<stdlib.h>	//strlen
#include<sys/socket.h>
#include<arpa/inet.h>	//inet_addr
#include<unistd.h>	//write
#include<pthread.h> //for threading , link with lpthread
#include <dirent.h> //For directories
#include <netdb.h> //For gethostbyname()
#include <openssl/evp.h> //For MD5()
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <time.h> //For cache timing

#define HASH(a) ((a) % 4)

FILE *file;
long long result;
int size;

void *connection_handler(void *server_number);
void put_command(char* filename);
void get_command();
void list_command();
void compute_md5_hash(const char *input, unsigned long long *md5_hash);

int main(int argc , char *argv[])
{
	/* 
   * check command line arguments 
   */
  	if (argc != 3) {
    	fprintf(stderr, "usage: %s <command> <filename> \n", argv[0]);
    	exit(1);
  	}

    if (strcasecmp(argv[1], "put") ==0){
        put_command(argv[2]);
    }
    else if (strcasecmp(argv[1], "get") ==0){
        get_command();
    }
    else if (strcasecmp(argv[1], "list") ==0){
        list_command();
    }
    else{
        printf("INCORRECT COMMAND");
        exit(1);
    }

	
    while(1){

    }
	
}

void *connection_handler(void *server_number){
    int socket_desc , client_sock , c , *new_sock;
	struct sockaddr_in server , client;
	int portno; /* port to listen on */
    int number = *(int*) server_number;
    portno=10000 + number;
    int first_chunk, second_chunk, chunk_size;
    char buffer[2000];
    size_t bytes_read;

    printf("Enter Func%d\n\r",number);
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
    //Create socket
	socket_desc = socket(AF_INET , SOCK_STREAM , 0);
	if (socket_desc == -1)
	{
		printf("Could not create socket");
	}
	puts("Socket created");
    //Config Server
    server.sin_family = AF_INET;
	server.sin_port = htons( portno );
    struct hostent *server_name = gethostbyname("10.224.76.120");
    if (server_name == NULL) {
        fprintf(stderr,"Error, no such host\n");
        }
    bcopy((char *)server_name->h_addr, (char *)&server.sin_addr.s_addr, server_name->h_length);
    //Connect 
    if(connect(socket_desc, (struct sockaddr *)&server, sizeof(server))<0){
        perror("Connect failed");
    }

    chunk_size = size/4;
    first_chunk = first_chunk * chunk_size;
    printf("Chunk %d\n",first_chunk);
    send(socket_desc, "403 Forbidden\n" , strlen("403 Forbidden\n"),0);
    fseek(file, first_chunk,SEEK_SET);
    if((bytes_read = fread(buffer, 1, chunk_size, file))<=0){perror("Error Reading File");}
    rewind(file);
    printf("READ %zu\n", bytes_read);
    if((send(socket_desc, buffer, bytes_read, 0)) != bytes_read){perror("Error Sending Data");}

    second_chunk = second_chunk * chunk_size;
    printf("Chunk %d\n", second_chunk);
    send(socket_desc, "403 Forbidden\n" , strlen("403 Forbidden\n"),0);
    fseek(file, second_chunk,SEEK_SET);
    if((bytes_read = fread(buffer, 1, chunk_size, file))<=0){perror("Error Reading File");}
    printf("READ %zu\n", bytes_read);
    if((send(socket_desc, buffer, bytes_read, 0)) != bytes_read){perror("Error Sending Data");}
    fclose(file);
    close(socket_desc);
    
}

void put_command(char* filename){
    int cur_pos;
    long long md5;


    //Determine File
    file = fopen(filename, "rb");
	if (file == NULL){
		perror("Error opening file");
    }
    cur_pos = ftell(file);
    fseek(file, 0, SEEK_END);
    size = ftell(file);
    rewind(file);

    //Determine Hash
    compute_md5_hash(filename,&md5);
    result = md5%4; if (result < 0){result = result * -1;}
    printf("Result of %lld %% 4 = %lld\n", md5, result);

    //Call Threads an Send
    pthread_t dfs1,dfs2,dfs3,dfs4;		
    int *dfc1,*dfc2,*dfc3,*dfc4;
    dfc1 = malloc(1); dfc2 = malloc(1);dfc3 = malloc(1);dfc4 = malloc(1);
    *dfc1=1;*dfc2=2;*dfc3=3;*dfc4=4;
	if( pthread_create(&dfs1, NULL ,  connection_handler,(void *)dfc1) < 0)
	{
		perror("could not create dfs1");
	}
    // if( pthread_create(&dfs2, NULL ,  connection_handler,(void *)dfc2) < 0)
	// {
	// 	perror("could not create dfs2");
	// }
    // if( pthread_create(&dfs3, NULL ,  connection_handler,(void *)dfc3) < 0)
	// {
	// 	perror("could not create dfs3");
	// }
    // if( pthread_create(&dfs4, NULL ,  connection_handler,(void *)dfc4) < 0)
	// {
	// 	perror("could not create dfs4");
	// }
}
void get_command(){

}
void list_command(){

}

void compute_md5_hash(const char *input, unsigned long long *md5_hash) {
   EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned int md_len;
    unsigned char md_result[EVP_MAX_MD_SIZE];

    OpenSSL_add_all_algorithms();

    md = EVP_get_digestbyname("md5");
    if (!md) {
        fprintf(stderr, "Unknown message digest\n");
        exit(EXIT_FAILURE);
    }

    mdctx = EVP_MD_CTX_new();
    EVP_DigestInit_ex(mdctx, md, NULL);
    EVP_DigestUpdate(mdctx, input, strlen(input));
    EVP_DigestFinal_ex(mdctx, md_result, &md_len);
    EVP_MD_CTX_free(mdctx);
    // Convert the MD5 hash bytes to a number
    *md5_hash = 0;
    for (int i = 0; i < md_len; i++) {
        *md5_hash <<= 8;
        *md5_hash += md_result[i];
    }
}