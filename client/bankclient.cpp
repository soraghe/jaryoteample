//클라이언트 프로그램

#include "msq_client.h"
#include <signal.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

#define CLSIGNOUT 5

using namespace std;


void signalHandler(int signum);

int main(int argc, char const *argv[]) {

	cout << "___________________테스트케이스____________________" << endl;

	MsgClient client;

	key_t msq_key = ftok("/", 31);
	if(msq_key == -1){
		perror("ftok() error!(ipc 키생성 실패) : ");
		kill(getpid(), SIGINT);
	}
	//클라이언트용 메시지큐 생성
	int msq_client_id = msgget(msq_key, IPC_CREAT | 0666);
	if(msq_client_id == -1) {
		perror("msgget() error!(클라이언트 메시지큐 생성 실패) : ");
		kill(getpid(), SIGINT);
	}
	
	//시그널 제어방식 재정의
	signal(SIGINT, signalHandler);
	
	//서버와 통신
	while(1) {
		//메시지 타입 구조체 초기화
		memset(&client, 0x00, sizeof(MsgClient));

		cout<< "\n----------BANK SERVICE----------\n 1. 로그인\n 2. 회원가입\n 9. 종료\n >> ";
		int todo;
		cin >> todo;
		client.cmd = todo;
		
		//로컬 사용자 정보 : 로그인 시 이곳에 정보 저장됨
		ClientInfo localInfo;
		memset(&localInfo, 0x00, sizeof(ClientInfo));

		switch(client.cmd) {
			case CLSIGNIN: {//로그인
				//ID/PW 입력
				cout<<"ID : "; cin >> localInfo.clientId;
				cout<<"PW : "; cin >> localInfo.clientPw;
				cout.clear();cin.clear();

				//client에 ID와 PW정보를 담아 메시지 송신
				client.mtype = MSG_TYPE_CLIENT; 
				strcpy(client.data.clientId, localInfo.clientId);
				strcpy(client.data.clientPw, localInfo.clientPw);
				int sndSize = msgsnd(msq_client_id, &client, MSG_SIZE_CLIENT, 0);
				//전송 실패 시
				if(sndSize != 0) {
					perror("msgsnd() error!(로그인 - ID/PW 전송 실패) ");
					kill(getpid(), SIGINT);
				}
				
				//서버에서 로그인 승인 결과 메시지를 받음
				int rcvSize = msgrcv(msq_client_id, &client, MSG_SIZE_CLIENT, MSG_TYPE_CLIENT, 0);
				if(rcvSize == -1) {//메시지 수신 실패 시
					perror("msgrcv() error!(로그인 - 로그인 승인결과 수신 실패) ");
					kill(getpid(), SIGINT);
				}
				//메시지 수신 성공 시
				else {
					//메시지는 받았지만 로그인 실패
					if(client.is_error == true) {
						cout << "로그인 실패." << endl;
						memset(&client, 0x00, sizeof(MsgClient));//에러코드를 포함한 메시지타입 구조체 초기화
						break;
					}
					else {//로그인 성공
						//서버에서 가져온 회원정보를 로컬에 저장 
						strcpy(localInfo.clientName, client.data.clientName);
						strcpy(localInfo.clientResRegNum, client.data.clientResRegNum);
						strcpy(localInfo.clientResRegNum, client.data.clientAccountNum);
						localInfo.clientBalance = client.data.clientBalance;
						
						cout << localInfo.clientName << "님 안녕하세요." << endl;
					}
				}
				break;
			}
			case CLSIGNUP: {//회원가입
				//회원가입정보 입력	
				memset(&localInfo, 0x00, sizeof(ClientInfo));
				cout<<"ID : "; cin >> localInfo.clientId;		//ID는 중복 금지(구현 필요)
				cout<<"PW : "; cin >> localInfo.clientPw;		//PW는 중복 가능, 패스워드 확인 절차 추가 필요
				cout<<"NAME : "; cin >> localInfo.clientName;	
				cout<<"RESREGNUM : "; cin >> localInfo.clientResRegNum;

				cout.clear();cin.clear();
			
				//회원가입 정보를 서버에 전송
				memset(&client, 0x00, sizeof(MsgClient));
				client.mtype = MSG_TYPE_CLIENT;			//메시지 타입 지정
				strcpy(client.data.clientId, localInfo.clientId);	//client구조체 멤버에 회원가입 정보 저장
				strcpy(client.data.clientPw, localInfo.clientPw);
				strcpy(client.data.clientName, localInfo.clientName);
				strcpy(client.data.clientResRegNum, localInfo.clientResRegNum);
				client.cmd = 2;
				//client.data.clientBalance = 0;
				int sndSize = msgsnd(msq_client_id, &client, MSG_SIZE_CLIENT, 0);	//전송
				//전송 실패 시
				if(sndSize != 0){
					perror("msgsnd() error!(회원가입 - 회원가입 정보 전송 실패) : ");
					kill(getpid(), SIGINT);
				}

				//회원가입 승인 여부 메시지 수신
				int rcvSize = msgrcv(msq_client_id, &client, MSG_SIZE_CLIENT, MSG_TYPE_CLIENT, 0);
				//메지시 수신 실패 시
				if(rcvSize == -1) {
					perror("msgrcv() error! (회원가입 - 회원가입 승인 결과 수신 실패) : ");
					kill(getpid(), SIGINT);
				}
				//메시지 수신 성공 시
				if(client.is_error == true) {//회원가입 실패
					cout << "회원가입 실패" << endl;
				} else {//회원가입 성공
					cout << client.data.clientName << " 님의 회원가입이 완료되었습니다." << endl;
					//서버에서 생성되는 계좌번호는 중복 금지(추후 이체기능을 위해)
					cout << " - 생성된 계좌번호 : " << client.data.clientAccountNum;
				}														
				//메시지타입 구조체 초기화(회원가입 != 로그인)
				memset(&client, 0x00, sizeof(MsgClient));
				memset(&localInfo, 0x00, sizeof(ClientInfo));
				break;
			}
			case 9: {
				cout << "안녕히 가십시오." <<endl;
				kill(getpid(), SIGINT);
				break;
			}
			default:{	//예상치 못한 오류
				cout << "잘못된 입력. 다시 입력하세요." << endl;
				break;
			}	
		}
		//로그인을 했다면
		if(client.cmd == CLSIGNIN) {
			while(1) {
				cout<< "\n----------업무 선택----------\n 3. 입금\n 4. 출금\n 5.로그아웃\n >> ";

				//작업코드 입력
				cin >> todo;
				client.cmd = todo;
				cout.clear();cin.clear();

				//작업코드에 따라 구조체에 담을 정보의 조합이 달라짐
				//3 : 입금
				//4 : 출금
				switch(client.cmd){
					case CLDEPOSIT: {//입금
						//사용자의 계좌정보를 요청(동기화 목적으로)
						client.mtype = MSG_TYPE_CLIENT;
						int sndSize = msgsnd(msq_client_id, &client, MSG_SIZE_CLIENT, 0);
						//msgsnd() 예외처리
						if(sndSize != 0){
							perror("msgsnd() error!(입금 - 계좌정보 요청 실패)");
							break;
						}
						int rcvSize = msgrcv(msq_client_id, &client, MSG_SIZE_CLIENT, MSG_TYPE_CLIENT, 0);
						if(rcvSize == -1) {
							perror("msgrcv() error!(입금 - 계좌정보 조회 실패)");
							break;
						}
						//잔액 정보가 다르면 서버 기준에 맞춰 최신화
						if(client.data.clientBalance != localInfo.clientBalance)
							localInfo.clientBalance = client.data.clientBalance;
						
						cout << "계좌에 남아있는 금액 : " << (int)client.data.clientBalance << "원" << endl;
						//사용자 입력
						int deposit;//입금액
						cout << "입금하실 금액 : "; cin >> deposit;
						
						//입금할 금액을 client.data.clientBalance에 담아 구조체 메시지 송신(localInfo는 변경 X)
						client.mtype = MSG_TYPE_CLIENT;
						client.data.clientBalance = deposit;
						sndSize = msgsnd(msq_client_id, &client, MSG_SIZE_CLIENT, 0);
						//msgsnd() 예외처리
						if(sndSize != 0){
							perror("msgsnd() error!(입금 - 입금정보 서버 전달 실패)");
							break;
						}
						//서버에서 입금 처리 후 보내주는 정보 갱신메시지를 수신
						rcvSize = msgrcv(msq_client_id, &client, MSG_SIZE_CLIENT, MSG_TYPE_CLIENT, 0);
						//msgrcv() 에외처리
						if(rcvSize == -1) {
							perror("msgrcv() error!(입금 - 갱신된 잔액조회 실패)");
							break;
						}
						cout << "입금 후 잔액 : " << (int)client.data.clientBalance << "원" << endl;
						localInfo.clientBalance = client.data.clientBalance;	//잔액을 localInfo와 동기화
						break;
					}	


					case CLWITHDRAW: {//출금
						//사용자의 계좌정보를 요청(동기화 목적으로)
						client.mtype = MSG_TYPE_CLIENT;
						int sndSize = msgsnd(msq_client_id, &client, MSG_SIZE_CLIENT, 0);
						//msgsnd() 예외처리
						if(sndSize != 0){
							perror("msgsnd() error!(출금 - 계좌정보 요청 실패)");
							break;
						}

						int rcvSize = msgrcv(msq_client_id, &client, MSG_SIZE_CLIENT, MSG_TYPE_CLIENT, 0);
						if(rcvSize == -1) {
							perror("msgrcv() error!(출금 - 계좌정보 조회 실패)");
							break;
						}

						//잔액 정보가 다르면 서버 기준에 맞춰 최신화
						if(client.data.clientBalance != localInfo.clientBalance)
							localInfo.clientBalance = client.data.clientBalance;
						cout << "계좌에 남아있는 금액 : " << (int)client.data.clientBalance << "원" << endl;
						//사용자 입력
						int withdraw;//출금액
						cout << "출금하실 금액 : "; cin >> withdraw;
						//잔액 부족 시 작업 다시 선택
						if(withdraw > client.data.clientBalance) {
							cout << "잔액이 부족합니다." << endl;
							break;
						} 

						//출금할 금액을 client.data.clientBalance에 담아 구조체 메시지 송신
						client.mtype = MSG_TYPE_CLIENT;
						client.data.clientBalance = withdraw;
						sndSize = msgsnd(msq_client_id, &client, MSG_SIZE_CLIENT, 0);
						//msgsnd() 예외처리
						if(sndSize != 0){
							perror("msgsnd() error!(출금 - 출금정보 서버 전달 실패)");
							break;
						}

						//서버에서 출금 처리 후 보내주는 정보 갱신메시지를 수신
						rcvSize = msgrcv(msq_client_id, &client, MSG_SIZE_CLIENT, MSG_TYPE_CLIENT, 0);
						//msgrcv() 에외처리
						if(rcvSize == -1) {
							perror("msgrcv() error!(출금 - 갱신된 잔액조회 실패)");
							break;
						}
						cout << "출금 후 잔액 : " << (int)client.data.clientBalance << "원" << endl;
						//최신화된 잔액 정보를 로컬에 저장
						localInfo.clientBalance = client.data.clientBalance;
						break;
					}

					case CLSIGNOUT: { 
						cout << "로그아웃.\n" << endl;
						//사용했던 로컬, 메시지타입 구조체들 초기화
						memset(&localInfo, 0x00, sizeof(MsgClient));
						memset(&localInfo, 0x00, sizeof(ClientInfo));
						break;
					}


					default: {	//예상치 못한 오류
						cout << "잘못된 입력. 다시 입력하세요." << endl;
						break;
					}	
				}	
				cout.clear();cout << endl;
			}		
		}
	}
	//테스트케이스 종료
	return 0;
}

void signalHandler(int signum) {
	if(signum == SIGINT) { 
		exit(0);
	}
}