#include "bankDB.h"			
#include <iostream>
#include <fcntl.h>		//┐
#include <sys/stat.h>	//│
#include <sys/types.h>	//├── for file I/O using system call
#include <unistd.h>		//┘
#include <string>		
#include <cstring>		//strlen(),strcmp()
#include <cstdlib>		//rand()		┐
#include <ctime>		//time()		├── for making account number randomly
#include <sstream>		//stringstream	┘

using namespace std;

//고객정보 가져오기
void* pull_client_info(const char* ID) {
	if(strlen(ID) <= 1) return nullptr;

	ClientInfo* target = new ClientInfo();	//리턴값
	ssize_t rsize = 0;	//clientDB.dat에서 읽어들인 데이터의 크기

	//파일 열기(./clientsDB.dat)
	string clientDBpath = "DB_client.dat";//클라이언트 DB 경로
	int fd = open(clientDBpath.c_str(), O_RDONLY, 0644);
	if(fd == -1){
		perror("open() error!(파일 조회를 위한 파일 열기 실패) : ");
		return nullptr;
	}
	
	do {//파일의 끝까지 읽기
		memset(target, 0x00, sizeof(ClientInfo));//구조체 초기화
		rsize = read(fd, (ClientInfo *)target, sizeof(ClientInfo));
		if(rsize == -1){
			perror("read() error!(pull_client_info() - DB파일 읽기 실패) : ");
			return nullptr;
		}
		//인자로 들어온 ID 동일한 ID를 가진 정보가 파일에 있으면
		else if(strcmp((target)->clientId, ID) == 0) {
			close(fd);
			return target;//target 반환
		}
	} while(rsize > 0);

	close(fd);
	return nullptr;
}
//오버로딩(고객정보 가져오기)
void* pull_client_info(const ClientInfo& info) {
	ClientInfo* target = new ClientInfo();	//리턴값
	ssize_t rsize = 0;	//clientDB.dat에서 읽어들인 데이터의 크기

	//파일 열기(./clientsDB.dat)
	string clientDBpath = "DB_client.dat";//클라이언트 DB 경로
	int fd = open(clientDBpath.c_str(), O_RDONLY, 0644);
	if(fd == -1){
		perror("open() error!(파일 조회를 위한 파일 열기 실패) : ");
		return nullptr;
	}
	
	do {//파일의 끝까지 읽기
		memset(target, 0x00, sizeof(ClientInfo));//구조체 초기화
		rsize = read(fd, (ClientInfo *)target, sizeof(ClientInfo));
		if(rsize == -1){
			perror("read() error!(pull_client_info() - DB파일 읽기 실패) : ");
			return nullptr;
		}
		//ID 비교
		else if(strcmp((target)->clientId, info.clientId) == 0) {
			close(fd);
			return target;//target 반환
		}
		//이름 비교
		else if(strcmp((target)->clientName, info.clientName) == 0) {
			close(fd);
			return target;//target 반환
		}
		//계좌번호 비교
		else if(strcmp((target)->clientAccountNum, info.clientAccountNum) == 0) {
			close(fd);
			return target;//target 반환
		}
	} while(rsize > 0);

	close(fd);
	return nullptr;
}

//클라이언트 ID 중복검사
bool is_our_client(const char* ID) { return (pull_client_info(ID) != nullptr) ? true : false; }

//클라이언트 정보 추가
bool add_client_info(const ClientInfo& info) {
	//clientsDB.dat에 동일 아이디가 있으면
	if(is_our_client(info.clientId)) return false;

	ClientInfo newClient;	//새 고객정보
	memset(&newClient, 0x00, sizeof(ClientInfo));
	
	//주민번호까지는 그대로 복사
	if(strlen(info.clientId) < 1) return false;
	strcpy(newClient.clientId,info.clientId);
	if(strlen(info.clientPw) < 1) return false;
	strcpy(newClient.clientPw, info.clientPw);
	if(strlen(info.clientName) < 1) return false;
	strcpy(newClient.clientName, info.clientName);
	if(strlen(info.clientResRegNum) < 1) return false;
	strcpy(newClient.clientResRegNum, info.clientResRegNum);
	
	newClient.clientBalance = 0;

	//계좌번호는 랜덤 생성(16자리), 저장(계좌번호 예시 : 123-123456-1234567)
	stringstream ss;
	srand(time(NULL));

	int randNum = (rand()%2);
	ss << randNum;
	randNum = (rand()%3) + 1;
	for(int i=0;i<2;i++)
		ss << randNum;
	ss << '-';
	for(int i = 0; i < 6; i++)
		ss << (rand() % 9) + 1;
	ss << '-';
	for(int i = 0; i < 7; i++)
		ss << (rand()%9) + 1;
	
	ss >> newClient.clientAccountNum;

	//clientsDB.dat에 정보 저장
	string clientDBpath = "DB_client.dat";
	int fd = open(clientDBpath.c_str(), O_CREAT | O_APPEND | O_WRONLY, 0644);
	if(fd == -1){
		perror("open() error!(쓰기위해 파일 열기) : ");
		return false;
	}
	lseek(fd,0,SEEK_END);
	if(write(fd, &newClient, sizeof(ClientInfo)) == -1 ) {
		perror("write() error!(파일 쓰기 실패) : ");
		return false;
	}

	return true;
}

//클라이언트 정보 수정(덮어쓰기)
bool modify_client_info(const ClientInfo& info) {
	int fd = 0;
	ssize_t rsize = 0;
	ClientInfo* target = new ClientInfo();

	string clientDBpath = "DB_client.dat";//클라이언트 DB 경로
	fd = open(clientDBpath.c_str(), O_RDWR, 0644);	//읽기와 쓰기 권한 모두 허가한 채 open()
	if(fd == -1) {//파일 옇기 실패
		perror("open() error!(파일 조회를 위한 파일 열기 실패) : ");
		return false;
	}

	do {//파일의 끝까지 읽기
		memset(target, 0x00, sizeof(ClientInfo));
		rsize = read(fd, (ClientInfo *)target, sizeof(ClientInfo));
		if(rsize == -1) {//파일 읽기 실패
			perror("read() error(DB파일 읽기 실패) : ");
			return false;
		}
		else if(rsize == 0 ) {	//읽어들인 내용이 없을 때
			break;
		}
		else {
			if(strcmp(target->clientId, info.clientId) == 0) { //일치하는 ID를 발견하면
				//read()로 읽어버려서 다음 고객을 가리키고 있는 파일 offset위치 다시 앞으로 이동
				ssize_t lsize = lseek(fd, -sizeof(ClientInfo), SEEK_CUR);
				if(lsize == -1) {//offset 변경 성공
					perror("lseek() error!(offset 이동 실패) : ");
					return false;
				}
				//파일 덮어쓰기
				ssize_t wsize = write(fd, &info, sizeof(ClientInfo));
				if(wsize == -1 ) {//덮어쓰기 실패
					perror("write() error");
					return false;
				}
			}
		}
	} while(rsize > 0);

	close(fd);
	return true;//수정에 성공하면 true
}

//관리자 ID 중복검사
bool is_our_admin(const char* ID) {
	AdminInfo* target = new AdminInfo();	//리턴값
	ssize_t rsize = 0;

	//파일 열기
	string adminListpath = "DB_admin.dat";//읽어올 파일 경로
	int fd = open(adminListpath.c_str(), O_RDONLY, 0644);
	if(fd == -1){
		perror("open() error!(파일 조회를 위한 파일 열기 실패) : ");
		return false;
	}
	
	//파일의 끝까지 읽기
	do {
		memset(target, 0x00, sizeof(AdminInfo));//구조체 초기화
		rsize = read(fd, (MsgAdmin *)target, sizeof(AdminInfo));
		if(rsize == -1) {
			perror("read() error!(DB파일 읽기 실패) : ");
			return false;
		}
		//인자로 들어온 ID/PW와 동일한 ID/PW를 가진 정보가 파일에 있으면
		else if(rsize > 0 && strcmp(target->adminId, ID) == 0) {
			close(fd);
			return true;
		}
	} while(rsize > 0);

	close(fd);
	return false;	//파일의 끝까지 일치하는 정보가 없으면 false
}

//관리자정보 추가
bool add_admin_info(const AdminInfo& info) {
	//clientsDB.dat에 동일 아이디가 있으면 false
	if(is_our_admin(info.adminId) == true)
		return false;

	AdminInfo newAdmin;	//새 고객정보
	memset(&newAdmin, 0x00, sizeof(AdminInfo));
	
	//내용 복사
	strcpy(newAdmin.adminId, info.adminId);
	strcpy(newAdmin.adminPw, info.adminPw);

	//adminList.dat에 정보 저장
	string adminListpath = "DB_admin.dat";//읽어올 파일 경로
	int fd = open(adminListpath.c_str(), O_CREAT | O_APPEND | O_WRONLY, 0644);
	if(fd == -1){
		perror("open() error!(쓰기위해 파일 열기) : ");
		return false;
	}
	lseek(fd, 0, SEEK_END);
	if(write(fd, &newAdmin, sizeof(AdminInfo)) == -1 ) {
		perror("write() error!(파일 쓰기 실패) : ");
		return false;
	}
	return true;
}