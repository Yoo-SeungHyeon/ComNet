#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define BUF_SIZE 100 //최대 버퍼 크기
#define MAX_CLNT 256 //최대 클라이언트 수

//함수 선언
void * handle_clnt(void * arg);
void send_msg(char * msg, int len);
void error_handling(char * msg);

//전역변수 선언
int clnt_cnt=0;
int clnt_socks[MAX_CLNT];
pthread_mutex_t mutx;

int main(int argc, char *argv[])
{
	int serv_sock, clnt_sock; //서버 소켓과 클라이언트 소켓 선언
	struct sockaddr_in serv_adr, clnt_adr; //주소 구조체 선언
	int clnt_adr_sz; //클라이언트 주소 크기 변수 선언
	pthread_t t_id; //쓰레드 변수 선언
	
	// 프로그래 실행시 Port지정했는지 확인
	if(argc!=2) {
		printf("Usage : %s <port>\n", argv[0]);
		exit(1);
	}
	
	pthread_mutex_init(&mutx, NULL); //데이터 일관성을 위해 mutex 선언
	
	serv_sock=socket(PF_INET, SOCK_STREAM, 0); //소켓 생성. PF_INET = IPv4, SOCK_STREAM + 0 = TCP

	memset(&serv_adr, 0, sizeof(serv_adr)); //주소 구조체 전부 0으로 초기화
	serv_adr.sin_family=AF_INET;  //타입 선언. IPv4
	serv_adr.sin_addr.s_addr=htonl(INADDR_ANY); //주소 선언. 자기자신
	serv_adr.sin_port=htons(atoi(argv[1])); //포트 선언. 프로그램 실행때 입력한 값
	

	if(bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr))==-1) //bind후, 실패시 에러 메세지실행
		error_handling("bind() error");

	if(listen(serv_sock, 5)==-1) //listen 실행 및 실패시 에러 메세지 출력
		error_handling("listen() error");
	
	while(1)
	{
		clnt_adr_sz=sizeof(clnt_adr); //클라이언트 주소 크기 저장
		clnt_sock=accept(serv_sock, (struct sockaddr*)&clnt_adr,&clnt_adr_sz); //클라이언트 소켓 생성
		
		pthread_mutex_lock(&mutx); //mutex lock을 통해 변수 선점
		clnt_socks[clnt_cnt++]=clnt_sock; //클라이언트 배열에 클라이언트 소켓 저장
		pthread_mutex_unlock(&mutx); //mutex unlock을 통해 변수 접근 허용
	
		pthread_create(&t_id, NULL, handle_clnt, (void*)&clnt_sock); //클라이언트 별 쓰레드 및 handle_clnt 실행
		pthread_detach(t_id); //클라이언트 쓰레드 종료 후 자동 으로 자원 반환
		printf("Connected client IP: %s \n", inet_ntoa(clnt_adr.sin_addr)); //연결한 클라이언트의 IP 출력
	}
	close(serv_sock); //서버 소켓 종료
	return 0;
}
	
void * handle_clnt(void * arg)
{
	int clnt_sock=*((int*)arg); //클라이언트 소켓 정보 초기화
	int str_len=0, i; //길이 변수 선언
	char msg[BUF_SIZE]; //메세지 배열 선언

	if(1){
		char msg1[BUF_SIZE];
		char msg2[BUF_SIZE];
		read(clnt_sock, msg1, sizeof(msg1));
		sprintf(msg2, "%s join the chat\n", msg1);
		printf("%s", msg2);
		send_msg(msg2, strlen(msg2));
	}

	while((str_len=read(clnt_sock, msg, sizeof(msg)))!=0) //큐에 메세지가 존재한다면
		send_msg(msg, str_len); //모든 클라이언트에게 메세지 전송
	
	pthread_mutex_lock(&mutx); //데이터 선점
	for(i=0; i<clnt_cnt; i++)   // remove disconnected client
	{								//앞선 while문을 빠져 나왔다는건 소켓을 종료한다는 의미 = clnt_socks배열에서 빠져야 한다는 의미
		if(clnt_sock==clnt_socks[i])  //배열에 존재한다면
		{
			while(i <clnt_cnt-1) //해당 배열을 제외하고 한칸씩 당김
			{
				clnt_socks[i]=clnt_socks[i+1];
				  i++;

			}

			break;
		}
	}
	clnt_cnt--;
	pthread_mutex_unlock(&mutx); //데이터 접근 허용
	close(clnt_sock); //클라이언트 소켓 종료.
	return NULL;
}
void send_msg(char * msg, int len)   // send to all
{
	int i;
	pthread_mutex_lock(&mutx); //데이터 선점
	for(i=0; i<clnt_cnt; i++)
		write(clnt_socks[i], msg, len); //클라이언트 배열을 통해 모든 클라이언트에게 전송
	pthread_mutex_unlock(&mutx); //데이터 접근 허용
}
void error_handling(char * msg) //에러 메세지 출력 함수
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}