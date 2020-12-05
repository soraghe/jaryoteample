//메시지큐에 사용할 자료형 정의
//작업코드를 추후 열거형(enum)으로 작성해 가독성을 높일 예정


#ifndef __MYMSG_H__
#define __MYMSG_H__

#define LEN 20

//메시지큐 사용을 위한 매크로
#define MSG_TYPE_ADMIN 2
#define MSG_SIZE_ADMIN (sizeof(MsgAdmin) - sizeof(long))

//고객 정보 양식
typedef struct __ClientInfo {	//나중에 STL 사용을 위해 클래스로 변경할 예정
	char clientId[LEN];				//고객 ID
	char clientPw[LEN];				//고객 PW
	char clientName[LEN];			//고객 이름
	char clientResRegNum[LEN];		//고객 주민번호
	char clientAccountNum[LEN];		//고객 계좌번호
	double clientBalance;			//고객 계좌잔액
} ClientInfo;

//관리자 정보 양식 
typedef struct __AdminInfo{
	char adminId[LEN];
	char adminPw[LEN];
} AdminInfo;

//관리자와 서버가 통신할 메시지 형태
typedef struct __MsgAdmin {
	long mtype;						//= MSG_TYPE_ADMIN
	int cmd;						//작업 코드
	char adminId[20];				//관리자 ID
	char adminPw[20];				//관리자 PW
	struct __ClientInfo data; 		//고객 정보(추후 변경 가능)
	bool is_error;					//에러 토큰(정보 제공 거부나 각종 에러 상황시에 쓰임)
} MsgAdmin;

//관리자 작업코드
enum AdminOffer {
	ADSIGNIN = 1,	//로그인
	ADSEARCHCLIENT,//클라이언트 전체 조회
	ADMODIFYCLINFO,	//클라이언트 정보 수정
	ADSIGNUP		//회원가입
};

#endif