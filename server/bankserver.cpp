//서버 프로그램

#include "msq.h"
#include <signal.h>
#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <cctype>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

void signalHandler(int signum);

//메시지큐 생성과 동작에 필요한 key와 msq 지시자
key_t msq_key = 0;
int msq_id = 0;

int main(int argc, char const* argv[]) {

	MsgClient client;
	MsgAdmin admin;
	pid_t pid = 0;

	//ipc사용을 위한 키 획득
	msq_key = ftok("key", 35);
	//메시지큐 생성
	msq_id = msgget(msq_key, IPC_CREAT | 0777);
	
	//프로그램 중단 시 메시지큐를 종료하기 위한 시그널 핸들러
	signal(SIGINT, signalHandler);
	
	//자식프로세스를 생성하여 부모프로세스에서는 클라이언트와 통신, 자식프로세스에서는 관리자와 통신하도록 구성
	pid = fork();

	//fork() 예외처리
	if(pid < 0){
		perror("fork() error! ");
		exit(-1);
	}
	//부모프로세스 - 고객(클라이언트)와 통신
	if(pid > 0){
		wait(0);//좀비프로세스 방지용(추후 관리자통신 구현에 따라 수정 예정)
		cout<< "clientserver is working... "<< endl;
		while(1) {
			//client와 통신할 메시지 구조체 초기화
			memset(&client, 0x00, sizeof(MsgClient));
			
			//메시지 수신(type 우선순위 X, 메시지가 올 때까지 대기)
			int rcvSize = msgrcv(msq_id, &client, MSG_SIZE_CLIENT, 0, 0);
			//msgrcv() 예외처리
			if(rcvSize == -1){
				perror("msgrcv() error! ");
				exit(-1);
			}
			//메시지 수신 성공 시
			if(rcvSize != -1) {
				//클라이언트에서 날아온 작업코드에 따라 다른 동작 수행
				//작업코드는 MsgClient.cmd에 저장되어있고, 클라이언트가 MsgClient구조체에 담아서 보내줌
				//1 : 고객 로그인
				//2 : 고객 회원가입
				//3 : 입금
				//4 : 출금
				//5 : 잔액조회(추후 추가 예정)
				switch(client.cmd){
					case 1:	{	//고객 로그인(임시 테스트)
								//날아온 메시지의 ID와 PW 일치 여부에 따라 정보를 제공하거나, 제공을 거부함 
								//(추후 DB파일에서 읽을 예정)
						//ID/PW가 일치하면
						if((strcmp(client.data.clientId, "hmschlng") == 0) && (strcmp(client.data.clientPw, "asdf1234!@") == 0)){			
							//MsgClient구조체에 정보를 담아 메시지 송신
							client.mtype = MSG_TYPE_CLIENT;
							strcpy(client.data.clientName, "이방환");
							strcpy(client.data.clientResRegNum, "940430-1");
							strcpy(client.data.clientAccountNum, "123123-01-987654");
							client.data.clientBalance = 315400;
							int sndSize = msgsnd(msq_id, &client, MSG_SIZE_CLIENT, 0);
							//msgsnd() 예외처리
							if(sndSize != 0){
								perror("msgsnd() error!(cmd=1) ");
								exit(0);
							}
						}
						//일치하지 않으면
						else{
							//(bool) MsgClient.is_error을 참으로 변경한 뒤 메시지 송신
							client.is_error = true;
							int sndSize = msgsnd(msq_id, &client, MSG_SIZE_CLIENT, 0);
							//msgsnd() 예외처리
							if(sndSize != 0){
								perror("msgsnd() error!(cmd=1) ");
								exit(0);
							}
						}
						break;
					}	
					case 2: {	//고객 회원가입
								//클라이언트에게서 ID/PW/이름/주민번호가 날아오면, 계좌번호를 생성, DB에 저장하고 클라이언트에게 메시지 송신

						//계좌번호 생성 후 메시지 송신
						//(계좌번호 생성은 추후 구현 예정)
						strcpy(client.data.clientAccountNum, "123456-12-1234567");	
						int sndSize = msgsnd(msq_id, &client, MSG_SIZE_CLIENT, 0);
						//msgsnd() 예외처리
						if(sndSize != 0){
							perror("msgsnd() error!(cmd=2) ");
							exit(0);
						}
						break;
					}
					case 3: {		//고객 입금
									//입금할 액수가 MsgClient.data.clientBalance에 담겨서 메시지가 날아옴
									//DB에 추가하는 부분은 추후 구현 예정
									
						cout << client.data.clientBalance << endl;	//입금할 액수 출력
						break;
					}
					case 4: {		//고객 출금
									//출금할 액수가 MsgClient.data.clientBalancl에 담겨서 메시지가 날아옴
									//DB에 추가하는 부분은 추후 구현 예정
						cout << client.data.clientBalance << endl;
						break;
					}
					case 5:		//고객 잔액조회(추후 구현예정)
					default: {	//예상치 못한 오류
						cout<<"에러 : 예상치 못한 클라이언트의 작업 요청."<<endl;
						exit(0);
					}
				}
			}
			cout.clear();	//ostream 초기화
		}
	}

	if(pid == 0){	//자식 프로세스(관리자와 통신)
		cout<<"adminserver is working..";	//추후 구현 예정
		/*
		while(1){
			memset(&admin, 0x00, sizeof(MsgAdmin));

			int adminMsgSize = msgrcv(msq_id, &admin, MSG_SIZE_ADMIN, MSG_TYPE_ADMIN, 0);
			if(adminMsgSize != -1) {//메시지 수신에 성공하면
				
				//관리자프로그램에서 날아온 작업 코드(admin.cmd)에 따라 해당 동작 수행
				switch(admin.cmd){
				case 1:		//관리자 회원가입
				case 2:		//관리자 로그인
				case 3:		//관리자 고객정보 조회
				case 4:		//관리자 고객정보 수정요청
				default:	//예상치 못한 오류
				}
			}
		}
		cout.clear();*/
	}

	return 0;
}

void signalHandler(int signum) {
	if(signum == SIGINT) {
		msgctl(msq_id, IPC_RMID, NULL);
		exit(0);
	}
}