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
		while(1){
			memset(&admin, 0x00, sizeof(MsgAdmin));

			int rcvSize = msgrcv(msq_id, &admin, MSG_SIZE_ADMIN, MSG_TYPE_ADMIN, 0);
			if(rcvSize == -1){
				perror("msgrcv() error! ");
				exit(-1);
			}
			else if(rcvSize != -1) {//메시지 수신에 성공하면

				//관리자프로그램에서 날아온 작업 코드(admin.cmd)에 따라 해당 동작 수행
				switch(admin.cmd){
					case 1:	//관리자 등록
						{
							admin.admin_login = false;
							int sndSize = msgsnd(msq_id, &admin, MSG_SIZE_ADMIN, 0);
							//msgsnd() 예외처리
							if(sndSize != 0){
								perror("msgsnd() error!(cmd=1) ");
								exit(0);
							}
							cout << "관리자 등록 완료" << endl;
							break;

							//데이터 베이스에 관리자 저장 코드 생성

						}
					case 2:		//관리자 로그인
						{  // 관리자 DB통해 로그인하려는 아이디와 비밀번호 부여
							// 일치할경우 플래그 생성해서 관리자 권한 부여
							if((strcmp(admin.adminId, "kim") == 0) && (strcmp(admin.adminPw, "1234") == 0)){
								//MsgClient구조체에 정보를 담아 메시지 송신
								admin.admin_login = true;            
								int sndSize = msgsnd(msq_id, &admin, MSG_SIZE_ADMIN, 0);
								cout << "관리자 로그인 완료" << endl;
								//msgsnd() 예외처리
								if(sndSize != 0){
									perror("msgsnd() error!(cmd=1) ");
									exit(0);
								}
							}
							//일치하지 않으면
							else{
								//(bool) MsgClient.is_error을 참으로 변경한 뒤 메시지 송신
								admin.admin_login = false;
								int sndSize = msgsnd(msq_id, &admin, MSG_SIZE_ADMIN, 0);
								//msgsnd() 예외처리
								if(sndSize != 0){
									perror("msgsnd() error!(cmd=1) ");
									exit(0);
								}
								cout << "관리자 로그인 실패" <<endl;  
							}
							break;
						}
					case 3:		//관리자 고객정보 조회
						{ 
							puts("고객정보 출력 요청");
							//권한이 있을 경우
							if (admin.admin_login ==  true) 
							{
								int i = 0;
								ifstream myfile("data.txt");
								ifstream smyfile("data.txt");
								char  ClientInfo[256];

								//파일 오픈
								if(myfile.is_open())
								{
									while(!myfile.eof())
									{
										myfile.getline(ClientInfo, 256);
										i++;
									}
								}
								myfile.close();
								admin.mtype = MSG_TYPE_ADMIN;
								admin.clientCnt = i;
								i = 0;
								int sndSize = msgsnd(msq_id, &admin, MSG_SIZE_ADMIN, 0);
								//msgsnd() 예외처리
								if(sndSize != 0){
									perror("msgsnd() error!(cmd=1) ");
									exit(0);
								}

								if(smyfile.is_open())
								{				 
									admin.mtype = MSG_TYPE_ADMIN;
									while(!smyfile.eof())
									{
										smyfile.getline(ClientInfo , 256);
										strcpy( admin.ClientInfo  , ClientInfo);
										int sndSize = msgsnd(msq_id, &admin, MSG_SIZE_ADMIN, 0);         
										//msgsnd() 예외처리
										if(sndSize != 0){
											perror("msgsnd() error!(cmd=1) ");
											exit(0);
										}
										i++;
									}
								}
								smyfile.close(); 
							}
							else 
							{
								admin.mtype = MSG_TYPE_ADMIN;
								admin.admin_login = false;
								int sndSize = msgsnd(msq_id, &admin, MSG_SIZE_ADMIN, 0);
								//msgsnd() 예외처리
								if(sndSize != 0){
									perror("msgsnd() error!(cmd=1) ");
									exit(0);
								}
							}
							break;
							//관리자 권한이 있으면 관리자 고객정보 전송
						}
					case 4:		//관리자 고객정보 수정요청
						{
							//관리자 권한이 있으면 관리자 고객정보 수정
							puts("고객정보 출력 요청");
							fstream myfile("data.txt");
							char  ClientInfo[256];
							char  tok_client[256];
							char* tok;
							int index = 0;
							if(myfile.is_open())
							{/*
								while(!myfile.eof())
								{
								 myfile.getline(ClientInfo, 256);
								 strcpy( tok_client,ClientInfo);
								 tok = strtok(tok_client, " ");
								 if(admin.data.clientId == tok)
								 {
								  index = tok_client.find(admin.data.clientName);
								  if(index == string::npos) break;
								  tok_client.replace(index , strlen(admin.data.clientName) , admin.data.clientPw);
								  strcpy(ClientInfo , tok_client);
								  cout << ClientInfo << endl;
								 } 
								}
								*/
							}
							myfile.close();


							
						break;
						}
					default:	//예상치 못한 오류
						{
						}
				}			
			}
		}
		cout.clear();
	}

	return 0;
}

void signalHandler(int signum) {
	if(signum == SIGINT) {
		msgctl(msq_id, IPC_RMID, NULL);
		exit(0);
	}
}
