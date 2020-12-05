#include "msq.h"
#include <signal.h>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

void signalHandler(int signum);

int main(int argc, char const *argv[]) {

	MsgAdmin admin;
	key_t msq_key = ftok("/", 31);
	if(msq_key == -1){
		perror("ftok() error!(ipc 키생성 실패) : ");
		kill(getpid(), SIGINT);
	}

	int msq_id = msgget(msq_key, IPC_CREAT | 0666);
	if(msq_id == -1) {
		perror("msgget() error!(관리자 메시지큐 생성 실패) : ");
		kill(getpid(), SIGINT);
	}

	//시그널 제어방식 재정의
	signal(SIGINT, signalHandler);
	memset(&admin, 0x00, sizeof(MsgAdmin));

	AdminInfo admin_localInfo;
	memset(&admin_localInfo, 0x00, sizeof(AdminInfo));

	bool admin_login = false;
	while(1)
	{
		//메시지 타입 구조체 초기화

		cout << "===작업 선택===\n1.관리자 로그인\n2.클라이언트 정보 조회\n3.클라이언트 정보수정\n4.관리자 등록\n5.관리자 로그아웃\n6.프로그램 종료\n";
		cout << ">>>>";
		cin >> admin.cmd;
		switch(admin.cmd)
		{
			case ADSIGNIN : {  
						char admin_id[20] = {0}; 
						char admin_pw[20] = {0};

						//사용자 입력
						cout << "\n----- 관리자 로그인 -----" << endl;
						cout << "관리자 ID : ";
						cin >> admin_localInfo.adminId;                //테스트 id = "admin1234"
						cout << "관리자 PW : ";
						cin >> admin_localInfo.adminPw;                //테스트 pw = "asdf1234!@"

						cout.clear();cin.clear();

						//admin에 ID와 PW정보를 담아 메시지 송신
						admin.mtype = MSG_TYPE_ADMIN;
						strcpy(admin.adminId, admin_localInfo.adminId);
						strcpy(admin.adminPw, admin_localInfo.adminPw);
						int sndSize = msgsnd(msq_id, &admin, MSG_SIZE_ADMIN, 0);
						//msgsnd() 예외처리
						if(sndSize != 0){
							perror("msgsnd() error!(로그인 - ID/PW 전송 실패) ");
							kill(getpid(), SIGINT);
						}

						//메시지 수신(관리자 로그인 승인 메시지 수신)
						int rcvSize = msgrcv(msq_id, &admin, MSG_SIZE_ADMIN, MSG_TYPE_ADMIN, 0);
						//msgrcv() 에외처리
						if(rcvSize == -1) {
							perror("msgrcv() error!(로그인 - 관리자정보 수신 실패) ");
							kill(getpid(), SIGINT);
						}
						//로그인 승인 결과 수신 성공 시 결과에 따라 메시지 출력
						else if(rcvSize != -1) {
							if(admin.is_error == true){
								cout << "로그인 실패.\n" << endl;
								//로그인 실패시 초기화
								memset(&admin, 0x00, sizeof(MsgAdmin));
								break;
							}
							else{
								admin_login = true;
								cout << "관리자 " << admin.adminId << "님 안녕하세요.\n" << endl;
							}
						}

						break;
					}
			case ADSEARCHCLIENT : {       
						      if(admin_login == true){
							      //클라이언트 조회
							      int num;  // 선택할 변수
							      char admin_data_id[20] = {0};
							      char admin_data_name[20] = {0};
							      char admin_data_account[20] = {0};

							      cout << "\n--- 클라이언트 정보 조회 ---\n1.ID로 찾기\n2.이름으로 찾기\n3.계좌번호로 찾기" << endl;
							      //admin에 수정할 정보를 담아 메시지 송신
							      cin >> num;
							      switch(num){
								      case 1 : {
										       cout << "검색할 ID 입력" <<endl << ">>>";
										       cin >> admin_data_id;
										       strcpy(admin.data.clientId , admin_data_id);
										       break;	
									       }
								      case 2 : {
										       cout << "검색할 이름 입력" <<endl << ">>>";
										       cin >> admin_data_name;
										       strcpy(admin.data.clientName , admin_data_name);
										       break;
									       }
								      case 3 : {
										       cout << "검색할 계좌번호 입력" << endl << ">>>";
										       cin >> admin_data_account;
										       strcpy(admin.data.clientAccountNum , admin_data_account);
										       break;
									       }
									       break;
							      }	
							      strcpy(admin.adminId, admin_localInfo.adminId);
							      admin.mtype = MSG_TYPE_ADMIN;
							      int sndSize = msgsnd(msq_id, &admin, MSG_SIZE_ADMIN, 0);
							      //msgsnd() 예외처리
							      if(sndSize != 0){
								      perror("msgsnd() error!(고객정보 조회 요청 실패) \n");
								      kill(getpid(), SIGINT);
							      }

							      int rcvSize = msgrcv(msq_id, &admin, MSG_SIZE_ADMIN, MSG_TYPE_ADMIN, 0);
							      //msgrcv() 에외처리
							      if(rcvSize == -1) {
								      perror("msgrcv() error!(고객 정보 수신 실패)\n ");
								      kill(getpid(), SIGINT);
							      }
							      //서버에서 보내는 정보 수수신 성공 시 결과에 따라 메시지 출력
							      else if(rcvSize != -1) {
								      if(admin.is_error == true){
									      cout << "고객정보 조회 실패." << endl;
									      break;
								      }
								      else{
									      cout << "\n검색된  고객정보" << endl;
									      cout << "고객 ID : " <<  admin.data.clientId << endl;
									      cout << "고객 비밀번호 : " << admin.data.clientPw << endl;
									      cout << "고객 이름 : " << admin.data.clientName << endl;
									      cout << "고객 주민등록 번호 : " << admin.data.clientResRegNum << endl;	
									      cout << "고객 계좌번호 : " << admin.data.clientAccountNum << endl;	
									      cout << "고객 계좌 잔액 : " <<admin.data.clientBalance << endl << endl;	           
								      }
							      }

						      }
						      else {
							      cout << "관리자 권한 없음, 로그인 해주세요\n" << endl;
						      }
						      break;

					      }
			case ADMODIFYCLINFO : {
						      if(admin_login == true){
							      int num;
							      char admin_data_id[20] = {0};
							      char admin_data_pw[20] = {0};
							      char admin_data_name[20] = {0};
							      char admin_data_account[20] = {0};

							      //클라이언트 정보 수정
							      cout << "\n--- 클라이언트 정보 수정 ---" << endl;
							      cout << "수정할 고객의 ID  입력\n>>> ";
							      cin >> admin_data_id;

							      //>>여기에 잘못된 ID입력에 대한 처리를 해주세요(server/bankserver.cpp 참고)


							      cout << "수정할 고객의 데이터 선택\n1.비밀번호\n2.고객 이름\n3.고객 계좌번호\n>>> ";
							      cin >> num;
							      switch(num)
							      {
								      case 1: {
										      cout << "수정할 비밀번호 입력\n>>> ";
										      cin >> admin_data_pw;
										      cout.clear();cin.clear();

										      //admin에 수정할 정보를 담아 메시지 송신
										      admin.mtype = MSG_TYPE_ADMIN;
										      strcpy(admin.data.clientId , admin_data_id);
										      strcpy(admin.data.clientPw, admin_data_pw);
										      int sndSize = msgsnd(msq_id, &admin, MSG_SIZE_ADMIN, 0);
										      //msgsnd() 예외처리
										      if(sndSize != 0) {
											      perror("msgsnd() error!(로그인 - ID/PW 전송 실패) ");
											      kill(getpid(), SIGINT);
										      }
										      int rcvSize = msgrcv(msq_id, &admin, MSG_SIZE_ADMIN, MSG_TYPE_ADMIN, 0);
										      //msgrcv() 에외처리
										      if(rcvSize == -1) {
											      perror("msgrcv() error!(고객 정보 수정 수신  실패) ");
											      kill(getpid(), SIGINT);
										      }
										      //고객정보 수정 수신 성공 시 결과에 따라 메시지 출력
										      else if(rcvSize != -1) {
											      if(admin.is_error == true){
												      cout << "고객정보 수정 실패(사유 : 해당 ID없음)\n" << endl;
												      break;
											      }
											      else{
												      cout << "고객정보 수정 완료.\n" << endl;
											      }
										      }
										      break;
									      }
								      case 2: {
										      cout << "수정할 고객 이름 입력\n>>> ";
										      cin >> admin_data_name;
										      cout.clear();cin.clear();

										      //admin에 수정할 정보를 담아 메시지 송신
										      admin.mtype = MSG_TYPE_ADMIN;
										      strcpy(admin.data.clientId , admin_data_id);
										      strcpy(admin.data.clientName, admin_data_name);
										      int sndSize = msgsnd(msq_id, &admin, MSG_SIZE_ADMIN, 0);
										      //msgsnd() 예외처리
										      if(sndSize != 0){
											      perror("msgsnd() error!(로그인 - ID/PW 전송 실패) ");
											      kill(getpid(), SIGINT);
										      }
										      int rcvSize = msgrcv(msq_id, &admin, MSG_SIZE_ADMIN, MSG_TYPE_ADMIN, 0);
										      //msgrcv() 에외처리
										      if(rcvSize == -1) {
											      perror("msgrcv() error!(고객 정보 수정 수신  실패) ");
											      kill(getpid(), SIGINT);
										      }
										      //고객정보 수정 수신 성공 시 결과에 따라 메시지 출력
										      else if(rcvSize != -1) {
											      if(admin.is_error == true){
												      cout << "고객정보 수정 실패(사유 : 해당 ID없음)\n" << endl;
												      break;
											      }
											      else{
												      cout << "고객정보 수정 완료.\n" << endl;
											      }
										      }
										      break;
									      }
								      case 3: {
										      cout << "수정할 고객 계좌번호 입력\n>>> ";
										      cin >> admin_data_account;
										      cout.clear();cin.clear();

										      //admin에 수정할 정보를 담아 메시지 송신
										      admin.mtype = MSG_TYPE_ADMIN;
										      strcpy(admin.data.clientId , admin_data_id);
										      strcpy(admin.data.clientAccountNum, admin_data_account);
										      int sndSize = msgsnd(msq_id, &admin, MSG_SIZE_ADMIN, 0);
										      //msgsnd() 예외처리
										      if(sndSize != 0){
											      perror("msgsnd() error!(로그인 - ID/PW 전송 실패) ");
											      kill(getpid(), SIGINT);
										      }
										      int rcvSize = msgrcv(msq_id, &admin, MSG_SIZE_ADMIN, MSG_TYPE_ADMIN, 0);
										      //msgrcv() 에외처리
										      if(rcvSize == -1) {
											      perror("msgrcv() error!(고객 정보 수정 수신  실패) ");
											      kill(getpid(), SIGINT);
										      }
										      //고객정보 수정 수신 성공 시 결과에 따라 메시지 출력
										      else if(rcvSize != -1) {
											      if(admin.is_error == true){
												      cout << "고객정보 수정 실패(사유 : 해당 ID없음)\n" << endl;
												      break;
											      }
											      else{
												      cout << "고객정보 수정 완료.\n" << endl;
											      }
										      }
										      break;
									      }
							      }
						      } else {

							      cout << "관리자 권한 없음. 로그인 해주세요.\n"<< endl;
						      }
						      break;
					      }	      
			case ADSIGNUP : {
						char admin_id[20] = {0}; 
						char admin_pw[20] = {0};

						if (admin_login==true) {
							//사용자 입력
							cout << "\n----- 관리자 등록 -----" << endl;
							cout << "관리자 ID : ";
							cin >> admin_id;                //테스트 id = "admin1234"
							cout << "관리자 PW : ";
							cin >> admin_pw;                //테스트 pw = "asdf1234!@"

							cout.clear();cin.clear();

							//admin에 ID와 PW정보를 담아 메시지 송신
							admin.mtype = MSG_TYPE_ADMIN;
							strcpy(admin.adminId, admin_id);
							strcpy(admin.adminPw, admin_pw);
							int sndSize = msgsnd(msq_id, &admin, MSG_SIZE_ADMIN, 0);
							//msgsnd() 예외처리
							if(sndSize != 0){
								perror("msgsnd() error!(로그인 - ID/PW 전송 실패) ");
								kill(getpid(), SIGINT);
							}

							//메시지 수신(관리자 등록 승인 메시지 수신)
							int rcvSize = msgrcv(msq_id, &admin, MSG_SIZE_ADMIN, MSG_TYPE_ADMIN, 0);
							//msgrcv() 에외처리
							if(rcvSize == -1) {
								perror("msgrcv() error!(로그인 - 관리자정보 수신 실패) ");
								kill(getpid(), SIGINT);
							}
							//관리자 등록  승인 결과 수신 성공 시 결과에 따라 메시지 출력
							else if(rcvSize != -1) {
								if(admin.is_error == true){
									cout << "관리자 정보 등록 실패." << endl;
									memset(&admin, 0x00, sizeof(MsgAdmin));
									break;
								}
								else{
									cout << "관리자 " << admin.adminId << " 등록 완료.\n" << endl;
								}
							}
							break;
						} else 
						{
							cout << "관리자 권한 없음. 로그인 해주세요\n" << endl;

						}
						break;
					}
			case 5 : {

					 cout << "관리자 로그아웃.\n" << endl;
					 admin_login = false;
					 break;
				 }
			case 6: {
					cout <<"프로그램 종료.\n" << endl;
					kill(getpid(), SIGINT);
				}
			default: {      //예상치 못한 오류
					 cout << "잘못된 입력. 다시 입력하십시오." << endl;
					 break;
				 }
		}
	}
}



void signalHandler(int signum) {
	if(signum == SIGINT) {
		exit(0);
	}
}

