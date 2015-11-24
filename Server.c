#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <time.h>

#define BUF_SIZE 128
#define MAX_CLNT 4
#define NAME_SIZE 20

void * handle_clnt(void * arg);
void send_msg(char * msg, int len);
void error_handling(char * msg);

int client_count=0;
int client_socks[MAX_CLNT];
char client_name[NAME_SIZE]= {NULL};
char client_names[MAX_CLNT][NAME_SIZE]= {NULL};


pthread_mutex_t mutx;

int main(int argc, char *argv[])
{
        int i;
	int server_sock, client_sock;        
	struct sockaddr_in server_address,client_address;
        int client_address_sz;
        
	pthread_t thread_id;
        
	if(argc!=2) {
                printf("%s <port> 로 입력하시오.\n", argv[0]);
                exit(1);
        }

        pthread_mutex_init(&mutx, NULL);
        // 쓰레드 초기화

        server_sock=socket(PF_INET, SOCK_STREAM, 0);
        //(IPv4,TCP/IP,0)

        memset(&server_address, 0, sizeof(server_address));
        // 서버IP 를 0으로 서버IP SIZE 만큼 초기화
        server_address.sin_family=AF_INET;
        // IPv4 설정
        server_address.sin_addr.s_addr=htonl(INADDR_ANY);
        //
        server_address.sin_port=htons(atoi(argv[1]));
        // 포트 설정

        if(bind(server_sock, (struct sockaddr*) &server_address, sizeof(server_address))==-1)
                error_handling("bind() error");

        // -1 이외 소켓식별자  -1 실패

        if(listen(server_sock, 5)==-1)
                error_handling("listen() error");

        // -1 이외 소켓식별자  -1 실패

        while(1)
        {
                client_address_sz=sizeof(client_address);
                // 클라이언트 IP SIZE

                client_sock=accept(server_sock, (struct sockaddr*)&client_address,&client_address_sz);
                // 서버소켓에 클라이언트IP의 접속요청을 받아들인다.

                if(client_count >= MAX_CLNT) {
                        printf("CONNECT FAIL : %d \n", client_sock);
                        write(client_sock, "too many users. sorry", BUF_SIZE);
                        continue;
                }
                //클라이언트 수가 최대 클라이언트 보다 많으면 표시

                pthread_mutex_lock(&mutx);
                // 다른 쓰레드 접근 제한

                client_socks[client_count]=client_sock;
                // 클라이언트로 받은 접속자 이름 입력

                read(client_sock, client_name, NAME_SIZE);
                // read ( 파일디스크립터,읽어들일 버퍼,버퍼크기);

                strcpy(client_names[client_count++], client_name);
                // 다음 클라이언트로부터 받은 접속자 이름입력

                pthread_mutex_unlock(&mutx);
                // 함수에 의하여 다른 쓰레드의 접근을 허락


                pthread_create(&thread_id, NULL, handle_clnt, (void*)&client_sock);
                //thread_id : 쓰레드의 ID
                //NULL : 쓰레드 특성(보통 0)
                //handle_clnt : 쓰레드가 실행할 함수
                //(void*)&client_sock : 쓰레드가 실행할 함수에 들어갈 인자값

                pthread_detach(thread_id);
                // 쓰레드 종료시 자원 반환
                printf("Connected client IP: %s \n", inet_ntoa(client_address.sin_addr));
                // 연결되면 출력
        }
        close(server_sock);
        // 서버소켓 종료
        return 0;
}

void * handle_clnt(void * arg)
{
        int i;
	int client_sock=*((int*)arg);
        int str_len=0;
        
	
	
        char msg[BUF_SIZE] = {NULL}; // 메세지 변수
      
	
		
        while((str_len=read(client_sock, msg, BUF_SIZE))!=0)
        // read ( 파일디스크립터,읽어들일 버퍼,버퍼크기);
        // client_sock 읽는다.
        {	  
			int j=0;
                        int noCli = 0;
                        int mGo = 0;
                        char tmpName[NAME_SIZE]= {NULL};
                        char tmpMsg[NAME_SIZE]= {NULL};

			send_msg(msg, str_len); // 메세지 전송 (이름+메세지)
                        read(client_sock, msg, BUF_SIZE); // 메세지 내용만 읽는다.
			read(client_sock, msg, BUF_SIZE); // 클라이언트 이름만 읽는다.			
                        printf("\n");
        }

        pthread_mutex_lock(&mutx);
	// 다른 쓰레드 접근 제한 

        for(i=0; i<client_count; i++)   // 클라이언트 종료시 종료된 클라이언트 변수 초기화 
        {
                if(client_sock==client_socks[i])
                {
                        while(i++<client_count-1) {
                                client_socks[i]=client_socks[i+1];
                                strcpy(client_names[i], client_names[i+1]);
                        }
                        break;
                }
        }
        client_count--;
        
	pthread_mutex_unlock(&mutx);
	// 다른 쓰레드 접근 제한 해제
        close(client_sock);
	// 소켓 종료
        return NULL;
}
void send_msg(char *msg, int len)   // send to all
{
        int i,j;

        pthread_mutex_lock(&mutx);
	// 다른 쓰레드 접근 제한

        for(i=0; i<client_count; i++)
        write(client_socks[i], msg, BUF_SIZE);
	// 클라이언트에게 메세지 전송

        pthread_mutex_unlock(&mutx);
	// 다른 쓰레드 접근 제한 해제
}
void error_handling(char * msg)
{
        fputs(msg, stderr);
        fputc('\n', stderr);
        exit(1);
}
