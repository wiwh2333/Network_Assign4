
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
long long store;
int size;
char command[100];
char chunk1[1000];
char chunk2[1000];
char chunk3[1000];
char chunk4[1000];
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex4 = PTHREAD_MUTEX_INITIALIZER;

void *connection_handler(void *server_number);
void put_command(char* filename);
void get_command(char* filename);
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
        strcpy(command, "put");
        put_command(argv[2]);
    }
    else if (strcasecmp(argv[1], "get") ==0){
        strcpy(command, "get");
        get_command(argv[2]);
    }
    else if (strcasecmp(argv[1], "list") ==0){
        strcpy(command, "list");
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
    int first_chunk, second_chunk, chunk_size, small_result, chunker;
    char buffer[10000], buffer2[10000];
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
    //Send command and filename(hashed)
    if((send(socket_desc, &command, sizeof(command), 0)) == -1){perror("Error Sending Data");}
    
    // Receive ACK
        if ((bytes_read = recv(socket_desc, buffer, sizeof(buffer), 0)) < 0) {
            perror("recv");
        }
        if (strcasecmp(buffer, "Ack") !=0){
            printf("ERROR IN ACK");
        }

    if (strcasecmp(command, "put") ==0){
        if((send(socket_desc, &store, sizeof(store), 0)) ==-1){perror("Error Sending Data");}
        // // Receive ACK
        if ((bytes_read = recv(socket_desc, buffer, sizeof(buffer), 0)) < 0) {
            perror("recv");
        }
        //Send Chunks
        chunk_size = size/4;
        chunker = size/4;
        if (first_chunk == 3){chunk_size = chunk_size + size%4;}
        first_chunk = first_chunk * chunker;
        
        fseek(file, first_chunk,SEEK_SET);
        if((bytes_read = fread(buffer, 1, chunk_size, file))<=0){perror("Error Reading File");}
        rewind(file);
        printf("Chunk%d,Buf%d,1st%d,number%d\n", chunk_size,bytes_read,first_chunk,number);
        if((send(socket_desc, buffer, bytes_read, 0)) != bytes_read){perror("Error Sending Data");}
        for (volatile int i = 0; i < 9999; i++){ if (i%3==0){ i++;}}
        chunk_size = size/4;
        if (second_chunk == 3){chunk_size = chunk_size + size%4;}
        second_chunk = second_chunk * chunker;
        
        fseek(file, second_chunk,SEEK_SET);
        if((bytes_read = fread(buffer, 1, chunk_size, file))<=0){perror("Error Reading File");}
        rewind(file);
        // // // Receive ACK
        // if ((bytes_read = recv(socket_desc, buffer, sizeof(buffer), 0)) < 0) {
        //     perror("recv");
        // }
        printf("Chunk2 %d\n", chunk_size);
        if((send(socket_desc, buffer, bytes_read, 0)) != bytes_read){perror("Error Sending Data");}
        //fclose(file);
    }
    else if(strcasecmp(command, "get") ==0){
        if((send(socket_desc, &store, sizeof(store), 0)) ==-1){perror("Error Sending Data");}
        // Receive the 1st message
        if ((bytes_read = recv(socket_desc, buffer, sizeof(buffer), 0)) < 0) {
            perror("recv");
        }
        printf("GOT\n");
        //ACK
        if (send(socket_desc, "Ack", sizeof("Ack"), 0) == -1) {
            perror("Error sending data");
            exit(EXIT_FAILURE);
        }
        printf("ACK\n");
        // Receive the 2nd message
        if ((bytes_read = recv(socket_desc, buffer2, sizeof(buffer2), 0)) < 0) {
            perror("recv");
        }
        printf("TOM\n");
        fflush(stdout);
        switch(first_chunk){
            case 0:
                pthread_mutex_lock(&mutex1);
                memcpy(chunk1,buffer,1000);
                pthread_mutex_unlock(&mutex1);
                break;
            case 1:
                pthread_mutex_lock(&mutex2);
                memcpy(chunk2,buffer,1000);
                pthread_mutex_unlock(&mutex2);
                break;
            case 2:
                pthread_mutex_lock(&mutex3);
                memcpy(chunk3,buffer,1000);
                pthread_mutex_unlock(&mutex3);
                break;
            case 3:
                pthread_mutex_lock(&mutex4);
                memcpy(chunk4,buffer,1000);
                pthread_mutex_unlock(&mutex4);
                break;
        }
        switch(second_chunk){
            case 0:
                pthread_mutex_lock(&mutex1);
                memcpy(chunk1,buffer2,1000);
                pthread_mutex_unlock(&mutex1);
                break;
            case 1:
                pthread_mutex_lock(&mutex2);
                memcpy(chunk2,buffer2,1000);
                pthread_mutex_unlock(&mutex2);
                break;
            case 2:
                pthread_mutex_lock(&mutex3);
                memcpy(chunk3,buffer2,1000);
                pthread_mutex_unlock(&mutex3);
                break;
            case 3:
                pthread_mutex_lock(&mutex4);
                memcpy(chunk4,buffer2,1000);
                pthread_mutex_unlock(&mutex4);
                break;
        }
    }
    close(socket_desc);
    printf("END\n");
    fflush(stdout);
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
    store = md5;
    printf("Result of %lld %% 4 = %lld,%d,%d\n", md5, result,size,size%4);

    //Call Threads an Send
    pthread_t dfs1,dfs2,dfs3,dfs4;		
    int *dfc1,*dfc2,*dfc3,*dfc4;
    dfc1 = malloc(1); dfc2 = malloc(1);dfc3 = malloc(1);dfc4 = malloc(1);
    *dfc1=1;*dfc2=2;*dfc3=3;*dfc4=4;
	if( pthread_create(&dfs1, NULL ,  connection_handler,(void *)dfc1) < 0)
	{
		perror("could not create dfs1");
	}
    if( pthread_create(&dfs2, NULL ,  connection_handler,(void *)dfc2) < 0)
	{
		perror("could not create dfs2");
	}
    if( pthread_create(&dfs3, NULL ,  connection_handler,(void *)dfc3) < 0)
	{
		perror("could not create dfs3");
	}
    if( pthread_create(&dfs4, NULL ,  connection_handler,(void *)dfc4) < 0)
	{
		perror("could not create dfs4");
	}
}
void get_command(char* filename){
    int cur_pos;
    long long md5;
    char store_name[21];

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
    store = md5;
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
    if( pthread_create(&dfs2, NULL ,  connection_handler,(void *)dfc2) < 0)
	{
		perror("could not create dfs2");
	}
    if( pthread_create(&dfs3, NULL ,  connection_handler,(void *)dfc3) < 0)
	{
		perror("could not create dfs3");
	}
    if( pthread_create(&dfs4, NULL ,  connection_handler,(void *)dfc4) < 0)
	{
		perror("could not create dfs4");
	}
    //Join
    // Join dfs1
if (pthread_join(dfs1, NULL) != 0) {
    perror("pthread_join failed for dfs1");
}

// Join dfs2
if (pthread_join(dfs2, NULL) != 0) {
    perror("pthread_join failed for dfs2");
}

// Join dfs3
if (pthread_join(dfs3, NULL) != 0) {
    perror("pthread_join failed for dfs3");
}

// Join dfs4
if (pthread_join(dfs4, NULL) != 0) {
    perror("pthread_join failed for dfs4");
}

    
    snprintf(store_name, sizeof(store_name), "%lld", store);
    // Open the output file in binary write mode
    // for (size_t i = 0; i < 100; i++) {
    //     printf("%02X ", (unsigned char)chunk1[i]);
    // }
    printf("\n");
    printf("Chunk1%s\n",chunk1);
    printf("Chunk2%s\n",chunk2);
    printf("Chunk3%s\n",chunk3);
    printf("Chunk4%s\n",chunk4);
    fflush(stdout);
    FILE* output = fopen(store_name, "wb");
     // Write chunk1 to the output file
    fwrite(chunk1, 1, 9, output);

    // Write chunk2 to the output file
    fwrite(chunk2, 1, 9, output);

    // Write chunk2 to the output file
    fwrite(chunk3, 1, 9, output);

    // Write chunk2 to the output file
    fwrite(chunk4, 1, 9, output);

    // Close the output file
    fclose(output);


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