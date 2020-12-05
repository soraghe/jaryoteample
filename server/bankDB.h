#ifndef __BANKDB_H__
#define __BANKDB_H__

#include "msq_server.h"
#include <iostream>

using namespace std;

/*클라이언트 관련 함수*/

//clientsDB.dat 파일에서 고객 정보를 읽어오는 함수
//- 사전 정의: 비어있지 않은 클라이언트의 ID
//- 사후 정의: 
//		(1) clientsDB.dat에 있는 ID와 일치하는 고객 정보를 반환
//		(2) ID가 공백이거나, 파일 내에 일치하는 ID가 없을 경우, nullptr 반환
//		(3) 오류 발생 시, nullptr 반환
void* pull_client_info(const char* ID);	

//clientsDB.dat 파일에서 고객 정보를 읽어오는 함수
//- 사전 정의: 클라이언트의 정보
//- 사후 정의: 
//		(1) info에 들어있는 정보 중(ID or 이름 or 계좌번호) clientsDB.dat의 내용과 일치하는 고객 정보를 반환
//		(2) info가 공백이거나, 파일 내에 일치하는 ID가 없을 경우, nullptr 반환
//		(3) 오류 발생 시, nullptr 반환
void* pull_client_info(const ClientInfo& info);

//clientsDB.dat 파일 내에 같은 ID가 존재하는 지 검사하는 함수 
//- 사전 정의: 비어있지 않은 클라이언트의 ID
//- 사후 정의:
//		(1) 파일에 있는 ID와 일치하는 ID가 있으면, true
//		(2) 파일 내에 일치하는 ID가 없거나 오류 발생 시, false
bool is_our_client(const char* ID);

//clientsDB.dat 파일에 새로운 고객정보를 추가하는 함수 
//- 사전 정의: 추가하려는 고객 정보가 담긴 ClientInfo 구조체
//- 사후 정의: 
//		(1) 성공하면, true
//		(2) 파일 내에 이미 ID가 있거나 회원정보 누락 시, false
//		(3) 오류 발생 시, false
bool add_client_info(const ClientInfo& info);

//기존 고객정보의 데이터를 수정하는 함수 
//- 사전 정의: 추가하려는 고객 정보가 담긴 ClientInfo 구조체
//- 사후 정의: 
//		(1) 성공하면, true
//		(3) 실패하면 false
bool modify_client_info(const ClientInfo& info);		

/*관리자 관련 함수*/

//adminList.dat 파일 내에 같은 관리자계정이 존재하는 지 검사하는 함수 
//- 사전 정의: 비어있지 않은 관리자의 ID
//- 사후 정의:
//		(1) 파일에 있는 ID와 일치하는 ID가 있으면, true
//		(2) 파일 내에 일치하는 ID가 없거나 오류 발생 시, false
bool is_our_admin(const char* ID);				

//adminList.dat 파일에 새로운 고객정보를 추가하는 함수 
//- 사전 정의: 추가하려는 고객 정보가 담긴 AdminInfo 구조체
//- 사후 정의: 
//		(1) 성공하면, true
//		(2) 파일 내에 이미 ID가 있거나 회원정보 누락 시, false
//		(2) 오류 발생 시, false
bool add_admin_info(const AdminInfo& info);			



#endif