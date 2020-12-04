#include "msq.h"
#include <iostream>
#include <cstdlib>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <cstring>

using namespace std;

int main(int argc, char const* argv[]) {
	int fd = 0;
	ssize_t rsize = 0;
	ClientInfo* cl = new ClientInfo();

	string clientDBpath = "./DB_client.dat";
	fd = open(clientDBpath.c_str(), O_RDONLY, 0644);
		if(fd == -1){
			perror("open() error!(파일 조회를 위한 파일 열기");
			exit(-1);
		}
	

	cout << "<clientDB.dat>" << endl;
	do {
		memset(cl, 0x00, sizeof(ClientInfo));
		rsize = read(fd, (ClientInfo *)cl, sizeof(ClientInfo));
		if(rsize == -1) {
			perror("read() error");
			exit(-1);
		}
		else if(strlen(cl->clientId) == 0 ){
			break;
		}
		else {
			cout << "ID: " << (cl->clientId) << " "
				 << "PW: " << (cl->clientPw) << " "
				 << "NAME: " << (cl->clientName) << " "
				 << "RESREG NUM: " << (cl->clientResRegNum) << " "
				 << "ACCOUNT NUM: " << (cl->clientAccountNum) << " "
				 << "BALANCE: " << (cl->clientBalance);
			cout << endl;
		}
		
	} while(rsize > 0);

	close(fd);
	return 0;
}
