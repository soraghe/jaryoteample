//서버 프로그램

#include "bankDB.h"
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
key_t msq_key = 0;//(=31)

//(메시지 큐를 관리자용과 클라이언트용으로 나누어 부모와 자식프로세스에서 각각 운용 -> 2개의 프로그램이나 다름없음)
int msq_client_id = 0;//클라이언트용
int msq_admin_id = 0;//관리자용

int main(int argc, char const* argv[]) {
	pid_t pid = 0;

	//ipc사용을 위한 키 획득
	msq_key = ftok("/", 31);
	if(msq_key == -1){
		perror("ftok() error!(ipc 키생성 실패) : ");
		kill(getpid(), SIGINT);
	}

	//시그널 동작 재정의
	signal(SIGINT, signalHandler);	//ctrl + c 입력 시그널
	signal(SIGUSR1, signalHandler);	//클라이언트 메시지큐 에러 시그널
	signal(SIGUSR2, signalHandler);	//관리자 메시지큐 에러 시그널
	
	//자식프로세스를 생성하여 부모프로//사용했던 로컬, 메시지타입 구조체들 초기화세스에서는 클라이언트와 통신, 자식프로세스에서는 관리자와 통신하도록 구성
	pid = fork();

	//fork() 예외처리
	if(pid < 0){
		perror("fork() error! ");
		exit(-1);
	}
	//부모프로세스 - 고객(클라이언트)와 통신
	if(pid > 0) {
		MsgClient client;
		//메시지큐 생성
		msq_client_id = msgget(msq_key, IPC_CREAT | 0666);
		if(msq_client_id == -1) {
			perror("msgget() error!(클라이언트 메시지큐 생성 실패) : ");
			kill(getpid(), SIGUSR1);
		}
		cout<< "clientserver is working... "<< endl;
		
		while(1) {
			//client와 통신할 메시지 구조체 초기화
			memset(&client, 0x00, sizeof(MsgClient));
			client.mtype = MSG_TYPE_CLIENT;
			
			//메시지 수신(type 우선순위 X, 메시지가 올 때까지 대기)
			int rcvSize = msgrcv(msq_client_id, &client, MSG_SIZE_CLIENT, MSG_TYPE_CLIENT, 0);
			//메시지 수신 실패 시
			if(rcvSize == -1) {
				perror("msgrcv() error!(작업 대기 중 - 클라이언트로부터 메시지수신 실패) : ");
				kill(getpid(), SIGUSR1);
			}
			//메시지 수신 성공 시
			if(rcvSize != -1) {
				//클라이언트에서 날아온 작업코드에 따라 다른 동작 수행
				//작업코드는 MsgClient.cmd에 저장되어있고, 클라이언트가 MsgClient구조체에 담아서 보내줌
				//1 : 고객 로그인
				//2 : 고객 회원가입
				//3 : 입금
				//4 : 출금
				switch(client.cmd){
					case CLSIGNIN:	{	//고객 로그인(임시 테스트)
										//날아온 메시지의 ID와 PW 일치 여부에 따라 정보를 제공하거나, 제공을 거부함 
						//DB에서 ID와 일치하는 고객정보를 읽어옴
						cout << "CLSIGNIN : (ID: " << client.data.clientId << ")"; 
						//ID가 DB에 존재하는지 검사
						if(is_our_client(client.data.clientId) == true) {//ID 일치
							ClientInfo* temp = (ClientInfo*)pull_client_info(client.data.clientId);
							//PW검사
							if(strcmp(client.data.clientPw, temp->clientPw) == 0) {//PW까지 일치하면
								//MsgClient구조체에 정보(더미 데이터)를 담아 메시지 송신
								strcpy(client.data.clientName, temp->clientName);
								strcpy(client.data.clientResRegNum, temp->clientResRegNum);
								strcpy(client.data.clientAccountNum, temp->clientAccountNum);
								client.data.clientBalance = temp->clientBalance;

								cout << "  >>  승인" << endl;
								client.is_error = false;
								if(msgsnd(msq_client_id, &client, MSG_SIZE_CLIENT, 0) != 0){
									perror("msgsnd() error!(CLSIGNIN - 회원정보 송신 실패) : ");
									kill(getpid(), SIGUSR1);
								}
								break;
							}
						}
						//ID/PW가 일치하지 않으면 로그인 거부 메시지 송신
						else if (is_our_client(client.data.clientId) == false) {
							//(bool) MsgClient.is_error을 참으로 변경한 뒤 메시지 송신
							cout << "  >>  거부" << endl;
							client.is_error = true;
							int sndSize = msgsnd(msq_client_id, &client, MSG_SIZE_CLIENT, 0);
							//송신 실패 시
							if(sndSize != 0) {
								perror("msgsnd() error!(CLSIGNIN - 로그인 거부 메시지 송신 실패) ");
								kill(getpid(),SIGUSR1);
							}
						}
						break;
					}	
					case CLSIGNUP: {	//고객 회원가입
										//클라이언트에게서 ID/PW/이름/주민번호가 날아오면, 계좌번호를 생성, DB에 저장하고 클라이언트에게 메시지 송신
						//ID가 DB 내의 ID와 중복되지 않으면
						cout << "새로운 회원가입 요청 : " << client.data.clientId;
						//기존 회원이 아니면
						if(is_our_client(client.data.clientId) == false) {
							add_client_info(client.data);	//DB에 회원정보 생성(계좌번호 생성)
							//DB에서 추가된 회원정보를 가져와서 메시지 전송(계좌번호 추가됨)
							ClientInfo* temp = (ClientInfo*)pull_client_info(client.data.clientId);
							strcpy(client.data.clientAccountNum, temp->clientAccountNum);
							int sndSize = msgsnd(msq_client_id, &client, MSG_SIZE_CLIENT, 0);
							if(sndSize != 0){//송신 실패 시
								perror("msgsnd() error!(CLSIGNUP - 회원가입 승인 메시지 송신 실패) ");
								kill(getpid(), SIGUSR1);
							}
							cout << "  >>  승인" << endl;
							cout << " - 생성된 계좌번호 : " << temp->clientAccountNum << endl;
						}
						else {//기존 회원이면
							//에러 토큰 활성화
							client.is_error = true;
							//메시지 전송
							int sndSize = msgsnd(msq_client_id, &client, MSG_SIZE_CLIENT, 0);
							if(sndSize != 0){//송신 실패 시
								perror("msgsnd() error!(CLSIGNUP - 회원가입 거부 메시지 송신 실패) ");
								kill(getpid(), SIGUSR1);
							}
							cout << "  >>  거부 : " << client.data.clientId << endl;
						}
						break;
					}
					case CLDEPOSIT: {	//고객 입금
						//고객정보를 전송(동기화 목적)
						ClientInfo* temp = (ClientInfo*)pull_client_info(client.data.clientId);				
						client.data.clientBalance = temp->clientBalance;
						cout << "CLDEPOSIT : (ID: " << client.data.clientId << ")";
						int sndSize = msgsnd(msq_client_id, &client, MSG_SIZE_CLIENT, 0);
						if(sndSize != 0) {
							perror("msgsnd() error!(CLDEPOSIT - 회원정보 송신 실패) : ");
							kill(getpid(), SIGUSR1);
						}
						else {//입금 금액정보 수신
							int rcvSize = msgrcv(msq_client_id, &client, MSG_SIZE_CLIENT, MSG_TYPE_CLIENT, 0);
							if(rcvSize == -1) {
								perror("msgrcv() error!(CLDEPOSIT - 입금액 정보 수신 실패) : ");
								kill(getpid(), SIGUSR1);
							}
							else { 
								//입금할 액수 출력
								cout << " : " << client.data.clientBalance << "원" << endl;	
								//DB에 있는 잔액 + 입금액
								temp = (ClientInfo*)pull_client_info(client.data.clientId);
								client.data.clientBalance += temp->clientBalance;
								//정보 수정
								modify_client_info(client.data);
								//client.data 갱신
								temp = (ClientInfo*)pull_client_info(client.data.clientId);
								client.data = *temp;
								//갱신 정보 전송
								int sndSize = msgsnd(msq_client_id, &client, MSG_SIZE_CLIENT, 0);
								if(sndSize != 0) {//전송 실패
									perror("msgsnd() error!(CLDEPOSIT - 갱신정보 송신 실패) : ");
									kill(getpid(), SIGUSR1);
								}
							}
						}
						break;
					}
					case CLWITHDRAW: {	//고객 출금
						cout << "CLWITHDRAW : (ID: " << client.data.clientId << ")";
						//고객정보를 전송(동기화 목적)
						ClientInfo* temp = (ClientInfo*)pull_client_info(client.data.clientId);
						client.data = *temp;
						int sndSize = msgsnd(msq_client_id, &client, MSG_SIZE_CLIENT, 0);
						if(sndSize != 0) {
							perror("msgsnd() error!(CLWITHDRAW - 회원정보 송신 실패) : ");
							kill(getpid(), SIGUSR1);
						}
						else {//입금 금액정보 수신
							int rcvSize = msgrcv(msq_client_id, &client, MSG_SIZE_CLIENT, MSG_TYPE_CLIENT, 0);
							if(rcvSize == -1) {
								perror("msgrcv() error!(CLWITHDRAW - 출금액 정보 수신 실패) : ");
								kill(getpid(), SIGUSR1);
							}
							else { 
								//입금할 액수 출력
								cout << " : " << client.data.clientBalance << "원" << endl;	
								//DB에 있는 잔액 + 입금액
								temp = (ClientInfo*)pull_client_info(client.data.clientId);
								//잔액 - 출금액
								client.data.clientBalance = temp->clientBalance - client.data.clientBalance;
								//정보 수정
								modify_client_info(client.data);
								//client.data 갱신
								temp = (ClientInfo*)pull_client_info(client.data.clientId);
								client.data.clientBalance = temp->clientBalance;
								//갱신 정보 전송
								int sndSize = msgsnd(msq_client_id, &client, MSG_SIZE_CLIENT, 0);
								if(sndSize != 0) {//전송 실패
									perror("msgsnd() error!(CLWITHDRAW - 갱신정보 송신 실패) : ");
									kill(getpid(), SIGUSR1);
								}
							}
						}
						break;
					}
					default: {	//예상치 못한 오류
						cout << "에러 : 예상치 못한 클라이언트의 작업 요청." << endl;
						kill(getpid(), SIGUSR1);
					}
				}
			}
			cout.clear();	//ostream 초기화
		}
	}
	//자식 프로세스(관리자와 통신)
	if(pid == 0) {	
		MsgAdmin admin;
		//메시지큐 생성
		msq_admin_id = msgget(msq_key, IPC_CREAT | 0666);
		if(msq_admin_id == -1) {
			perror("msgget() error!(관리자 메시지큐 생성 실패) : ");
			kill(getpid(), SIGUSR2);
		}
		cout << "adminserver is working.." << endl;	//추후 구현 예정
		//관리자는 DB파트가 구현이 되어야 기능을 추가할 수 있습니다.

		while(1){
			memset(&admin, 0x00, sizeof(MsgAdmin));
			admin.mtype = MSG_TYPE_ADMIN;

			int adminMsgSize = msgrcv(msq_admin_id, &admin, MSG_SIZE_ADMIN, MSG_TYPE_ADMIN, 0);
			if(adminMsgSize != -1) {//메시지 수신에 성공하면
				
				//관리자프로그램에서 날아온 작업 코드(admin.cmd)에 따라 해당 동작 수행
				switch(admin.cmd) {
					case ADSIGNIN: {			//관리자 로그인
						cout << "ADSIGNIN : (ID: " << admin.adminId << ")"; 
						//관리자DB에 있는 ID이면
						if(is_our_admin(admin.adminId) == true) {
							admin.is_error = false;
							int sndSize = msgsnd(msq_admin_id, &admin, MSG_SIZE_ADMIN, 0);
							if(sndSize != 0) {
								perror("msgsnd() error!(ADSIGNIN - 승인 메시지 전송 실패) ");
								kill(getpid(), SIGUSR2);
							}
							cout << "  >>  승인" << endl;
						}
						else {
							admin.is_error = true;
							int sndSize = msgsnd(msq_admin_id, &admin, MSG_SIZE_ADMIN, 0);
							if(sndSize != 0) {
								perror("msgsnd() error!(ADSIGNIN - 거부 메시지 전송 실패) ");
								kill(getpid(), SIGUSR2);
							}
							cout << "  >>  거부" << endl;
						}
						break;
					}
					case ADSEARCHCLIENT: {	//클라이언트 검색
						cout << "ADSEARCHCLIENT : (from " << admin.adminId << ")"; 
						ClientInfo* temp = (ClientInfo*)pull_client_info(admin.data);
						if(is_our_client(temp->clientId) == true) {
							admin.is_error = false;
							admin.data = *temp;
							int sndSize = msgsnd(msq_admin_id, &admin, MSG_SIZE_ADMIN, 0);
							if(sndSize != 0) {
								perror("msgsnd() error!(ADSEARCHCLIENT - 고객정보 전송 실패) : ");
							}
							cout << "  >>  전송완료(client ID : " << admin.data.clientId << ")" << endl;
						}
						else {
							admin.is_error = true;
							int sndSize = msgsnd(msq_admin_id, &admin, MSG_SIZE_ADMIN, 0);
							if(sndSize != 0) {
								perror("msgsnd() error!(ADSEARCHCLIENT - 거부 메시지 전송 실패) : ");
							}
							cout << "  >>  거부" << endl;
						}
						break;
					}
					case ADMODIFYCLINFO: {	//클라이언트 정보수정
						cout << "ADMODIFYCLINFO : (from " << admin.adminId << ")  (client ID : " << admin.data.clientId << ")";
						
						//존재하는 고객인지 먼저 체크

						if(is_our_client(admin.data.clientId) == false) {
							admin.is_error = true;
							int sndSize = msgsnd(msq_admin_id, &admin, MSG_SIZE_ADMIN, 0);
							if(sndSize != 0) {//거부 메시지 전송 실패
								perror("msgsnd() error!(ADMODIFYINFO - 거부 메시지 전송 실패) ");
								kill(getpid(), SIGUSR2);
							}
							cout << "  >>  거부" << endl;
						}
						else {//기존 고객이면
							ClientInfo* temp = (ClientInfo*)pull_client_info(admin.data.clientId);
							if(strcmp(admin.data.clientPw, "\0") != 0)
								strcpy(temp->clientPw, admin.data.clientPw);
							if(strcmp(admin.data.clientName, "\0") != 0)
								strcpy(temp->clientName, admin.data.clientName);
							if(strcmp(admin.data.clientAccountNum, "\0") != 0)
								strcpy(temp->clientAccountNum, admin.data.clientAccountNum);
							modify_client_info(*temp);
							admin.data = *temp;
							admin.is_error = false;
							int sndSize = msgsnd(msq_admin_id, &admin, MSG_SIZE_ADMIN, 0);
							if(sndSize != 0) {//거부 메시지 전송 실패
								perror("msgsnd() error!(ADMODIFYINFO - 수정 완료 메시지 전송 실패) ");
								kill(getpid(), SIGUSR2);
							}
							cout << "  >>  수정완료" << endl;
					//	}
						break;
					}
					case ADSIGNUP: {			//관리자 등록
						cout << "ADSIGNUP : (ID :" << admin.adminId << ")";
						//기존 회원이 아니면
						if(is_our_admin(admin.adminId) == false) {
							//ID/PW를 담아 DB에 추가
							AdminInfo newAdmin;
							strcpy(newAdmin.adminId, admin.adminId);
							strcpy(newAdmin.adminPw, admin.adminPw);
							add_admin_info(newAdmin);//관리자 정보를 DB에 추가
							if(is_our_admin(admin.adminId) == true) { //다시 확인해 추가가 되었으면
								admin.is_error = false;
								int sndSize = msgsnd(msq_admin_id, &admin, MSG_SIZE_ADMIN, 0);
								if(sndSize != 0) {//승인 메시지 전송 실패
									perror("msgsnd() error!(ADSIGNUP - 승인 메시지 전송 실패) : ");
									kill(getpid(),SIGUSR2);
								}
								cout<< "  >>  승인" << endl;
							}
						}
						else {//기존 회원이면
							admin.is_error = true;
							int sndSize = msgsnd(msq_admin_id, &admin, MSG_SIZE_ADMIN, 0);
							if(sndSize != 0) {//승인 메시지 전송 실패
								perror("msgsnd() error!(ADSIGNUP - 거부 메시지 전송 실패) : ");
								kill(getpid(),SIGUSR2);
							}
							cout<< "  >>  거부" << endl;
						}
						break;
					}
					default: {	//예상치 못한 오류
						cout << "잘못된 입력. 다시 입력하십시오." << endl;
						break;
					}
				}
			}//1 개의 요청 처리 완료
			cout.clear();
		}
	}

	return 0;
}

void signalHandler(int signum) {
	switch(signum){
		case SIGINT: {//인터럽트 시그널 수신 시 모든 메시지큐 종료 후 프로그램 종료.
			msgctl(msq_client_id, IPC_RMID, NULL);
			msgctl(msq_admin_id, IPC_RMID, NULL);
			exit(0);
		}
		case SIGUSR1: {//클라이언트에 문제 발생 시 메시지큐 제거
			msgctl(msq_client_id, IPC_RMID, NULL);

			exit(1);
		}
		case SIGUSR2: {//관리자에 문제 발생 시 메시지큐 제거
			msgctl(msq_admin_id, IPC_RMID, NULL);
			exit(2);
		}
	}
}
