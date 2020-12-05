//메시지큐에 사용할 자료형 정의
//작업코드를 추후 열거형(enum)으로 작성해 가독성을 높일 예정


#ifndef __MYMSG_H__
#define __MYMSG_H__

#define LEN 20

//메시지큐 사용을 위한 매크로
#define MSG_TYPE_CLIENT 1
#define MSG_SIZE_CLIENT (sizeof(MsgClient) - sizeof(long))

//고객 정보 양식
typedef struct __ClientInfo {	//나중에 STL 사용을 위해 클래스로 변경할 예정
	char clientId[LEN];				//고객 ID
	char clientPw[LEN];				//고객 PW
	char clientName[LEN];			//고객 이름
	char clientResRegNum[LEN];		//고객 주민번호
	char clientAccountNum[LEN];		//고객 계좌번호
	double clientBalance;			//고객 계좌잔액
} ClientInfo;

//고객과 서버가 통신할 메시지 형태
typedef struct __MsgClient {
	long mtype;						//= MSG_TYPE_CLIENT
	int cmd;						//작업 코드(메시지의 형태는 건들지 않고 이 작업코드로 다른 동작을 실행하도록 구성)
	struct __ClientInfo data;		//고객 정보
	bool is_error;					//에러 토큰(정보 제공 거부나 각종 에러 상황시에 쓰임)
} MsgClient;

//클라이언트 작업코드
enum ClientOffer {
	CLSIGNIN = 1,	//로그인
	CLSIGNUP,		//회원 가입
	CLDEPOSIT,		//입금
	CLWITHDRAW		//출금
};

#endif