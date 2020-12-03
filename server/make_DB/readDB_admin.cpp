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
	AdminInfo* ad = new AdminInfo();

	string adminDBpath = "./DB_admin.dat";
	fd = open(adminDBpath.c_str(), O_RDONLY, 0644);
		if(fd == -1){
			perror("open() error!(파일 조회를 위한 파일 열기");
			exit(-1);
		}
	

	cout << "<DB_admin.dat>" << endl;
	do {
		memset(ad, 0x00, sizeof(AdminInfo));
		rsize = read(fd, (AdminInfo *)ad, sizeof(AdminInfo));
		if(rsize == -1) {
			perror("read() error");
			exit(-1);
		}
		else if(strlen(ad->adminId) == 0 ){
			break;
		}
		else {
			cout << "ID: " << (ad->adminId) << " "
				 << "PW: " << (ad->adminPw) << " "
				 << endl;
		}
		
	} while(rsize > 0);

	close(fd);
	return 0;
}
