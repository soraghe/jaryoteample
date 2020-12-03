#include "msq.h"			//reference of Clientinfo 
#include <fcntl.h>	
#include <iostream>
#include <cstdlib>			//for make random number
#include <ctime>			//for designate the seed number
#include <list>				//for making a list of ClientInfo 
#include <string>
#include <cstring>			//for memset
#include <sstream>			//for using stringstream
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std;

int main(void) { 
	list <ClientInfo> clientList;

	srand(time(NULL));
	
	while(1) {
		//초기화
		ClientInfo newClient;
		memset(&newClient, 0x00, sizeof(ClientInfo));

		cout << "Input Id : ";
		cin >> newClient.clientId;

		cout << "Input Pw : ";
		cin >> newClient.clientPw;
		
		cout << "Input name : ";
		cin >> newClient.clientName;

		/*주민번호 (7자리)*/

		stringstream ss;				//stringstream 사용

		int randNum = 0;
		//생년
		for(int i=0;i<2;i++){
			randNum = (rand()%9) + 1; 
			ss << randNum;			//요런식으로 쓰면 문자열로 바로 만들어줌
		}
		//월
		randNum = (rand()%12) + 1;
		if(randNum<10)
			ss << 0;
		ss << randNum;
		//일(2월계산 귀찮아서 1-27까지로..ㅎ)
		randNum = (rand()%27) + 1;
		if(randNum<10)
			ss<<0;
		ss << randNum;
		//성별
		randNum = (rand()%2) + 1;
		ss << '-' << randNum;
		
		ss >> newClient.clientResRegNum;
		ss.clear();

		/*계좌번호 생성*/
		randNum = (rand()%2);
		ss << randNum;
		randNum = (rand()%3) + 1;
		for(int i=0;i<2;i++)
			ss << randNum;
		ss << '-';
		for(int i=0;i<6;i++)
			ss << (rand()%9) + 1;
		ss << '-';
		for(int i=0;i<7;i++)
			ss << (rand()%9) + 1;

		ss >> newClient.clientAccountNum;

		newClient.clientBalance = (((rand()%1000000)/1000) +1)*1000;
		cout << "< 입력한 정보 >" << endl;
		cout << newClient.clientId << " "
			 << newClient.clientPw << " "
			 << newClient.clientName << " "
			 << newClient.clientResRegNum << " "
			 << newClient.clientAccountNum << " "
			 << newClient.clientBalance
			 << endl;
		
		clientList.push_back(newClient);
		cout << ">> Successfully added to list!" << endl;
		cout << ">> Continue? (y/n) : ";
		char cmd;
		cin >> cmd;
		if(cmd == 'n') break;
	}

	string clientDBpath = "./DB_client.dat";
	int fd = open(clientDBpath.c_str(), O_CREAT | O_APPEND | O_WRONLY, 0644);
	if(fd == -1){
		perror("open() error!(쓰기위해 파일 열기) : ");
		return 1;
	}
	cout << "< list에 저장된 내용 >" <<endl;
	list<ClientInfo>::iterator iter;
	for(iter = clientList.begin(); iter != clientList.end(); iter++) {
		cout << (*iter).clientId << " "
		     << (*iter).clientPw << " "
		     << (*iter).clientName << " "
		     << (*iter).clientResRegNum << " "
		     << (*iter).clientAccountNum << " "
		     << (*iter).clientBalance << " " << endl;

		if(write(fd, &(*iter), sizeof(ClientInfo)) == -1 ) {
			perror("write() error");
			return 1;
		}
	}

	return 0;
}
