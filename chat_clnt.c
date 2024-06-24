#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h> 
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
	
#define BUF_SIZE 100 //최대 버퍼 크기
#define NAME_SIZE 20 //최대 이름 크기

//함수 호출
void * send_msg(void * arg);
void * recv_msg(void * arg);
void error_handling(char * msg);

//전역변수 선언
char name[NAME_SIZE]="[DEFAULT]";
char msg[BUF_SIZE];
	
int main(int argc, char *argv[])
{
	int sock; //서버와 연결할 소켓 변수 선언
	struct sockaddr_in serv_addr; //서버 정보 구조체 선언 (IP, Port 등등 선언)
	pthread_t snd_thread, rcv_thread; //Thread 타입 변수 선언
	void * thread_return;

	// 클라이언트 실행시 IP와 Port, Name을 입력했는지 확인.
	if(argc!=4) {
		printf("Usage : %s <IP> <port> <name>\n", argv[0]);
		exit(1);
	 }
	
	sprintf(name, "[%s]", argv[3]); //입력받은 Name을 이용해 전송할 메세지 앞에 넣을 별명 생성
	sock=socket(PF_INET, SOCK_STREAM, 0); //PF_INET = IPv4, SOCK_STREAM + 0 = TCP
	
	memset(&serv_addr, 0, sizeof(serv_addr)); //구조체의 모든 값 0으로 선언
	serv_addr.sin_family=AF_INET; //IP 종류 선언. IPv4
	serv_addr.sin_addr.s_addr=inet_addr(argv[1]); //IP지정
	serv_addr.sin_port=htons(atoi(argv[2])); //Port 지정
	  
	if(connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==-1) //연결 에러 발생시 실행
		error_handling("connect() error");
	
	pthread_create(&snd_thread, NULL, send_msg, (void*)&sock); //전송 쓰레드 생성
	pthread_create(&rcv_thread, NULL, recv_msg, (void*)&sock); //수신 쓰레드 생성
	pthread_join(snd_thread, &thread_return); //전송 쓰레드 종료되지 않게 Block
	pthread_join(rcv_thread, &thread_return); //수신 쓰레드 종료되지 않게 Block
	close(sock);  //소켓 종료
	return 0;
}
	
void * send_msg(void * arg)   // send thread main
{
	int sock=*((int*)arg); //소켓값으로 변수 초기화
	char name_msg[NAME_SIZE+BUF_SIZE]; //메세지 변수 선언
	while(1)
	{
		fgets(msg, BUF_SIZE, stdin); //메세지를 msg에 저장
		if(!strcmp(msg,"q\n")||!strcmp(msg,"Q\n")) //q나 Q입력시 종료
		{
			close(sock);
			exit(0);
		}
		sprintf(name_msg,"%s %s", name, msg); //name과 msg를 합쳐서 전송할 name_msg 생성
		write(sock, name_msg, strlen(name_msg)); //write를 통해 전송
	}
	return NULL;
}
	
void * recv_msg(void * arg)   // read thread main
{
	int sock=*((int*)arg); //소켓 파일 디스크립터 초기화
	char name_msg[NAME_SIZE+BUF_SIZE]; //메세지를 저장할 변수 선언
	int str_len; //길이 변수 선언
	while(1)
	{
		str_len=read(sock, name_msg, NAME_SIZE+BUF_SIZE-1); //name_msg에 값을 저장
		if(str_len==-1)  // -1 반환시 종료
			return (void*)-1;
		name_msg[str_len]=0; // 배열 제일 뒤에 0 추가. 출력할때 끝이라고 알려주는 역할
		fputs(name_msg, stdout); //화면에 출력
	}
	return NULL;
}
	
void error_handling(char *msg) //에러 발생시 에러 메세지를 통해 처리
{
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
