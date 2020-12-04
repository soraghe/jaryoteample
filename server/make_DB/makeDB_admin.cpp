#include "msq.h"			//reference of __client_info 
#include <iostream>
#include <cstdlib>			//for make random number
#include <list>				//for making a list of __client_info 
#include <string>
#include <cstring>			//for memset
#include <sstream>			//for using stringstream
#include <fcntl.h>	
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

int main(void) { 
	list <AdminInfo> adminList;

	while(1) {
		//초기화
		AdminInfo newAdmin;
		memset(&newAdmin, 0x00, sizeof(AdminInfo));

		//정보 입력
		//ID
		cout << "Input Id : ";
		cin >> newAdmin.adminId;
		//PW
		cout << "Input Pw : ";
		cin >> newAdmin.adminPw;
		
		adminList.push_back(newAdmin);
		
		cout << ">> Successfully added to list!" << endl;
		cout << ">> Continue? (y/n) : ";
		char cmd;
		cin >> cmd;
		if(cmd == 'n') break;
	}

	string adminListpath = "./DB_admin.dat";
	int fd = open(adminListpath.c_str(), O_CREAT | O_APPEND | O_WRONLY, 0644);
	if(fd == -1){
		perror("open() error!(쓰기위해 파일 열기) : ");
		return 1;
	}
	cout << "< list에 저장된 내용 >" <<endl;
	list<AdminInfo>::iterator iter;
	for(iter = adminList.begin(); iter != adminList.end(); iter++) {
		cout << (*iter).adminId << " "
		     << (*iter).adminPw
		     << endl;

		if(write(fd, &(*iter), sizeof(AdminInfo)) == -1 ) {
			perror("write() error");
			return 1;
		}
	}

	return 0;
}
