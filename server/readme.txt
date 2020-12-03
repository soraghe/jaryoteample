<서버 프로그램>
컴파일 경로 : bankserver.cpp bankDB.cpp bankDB.h msq.h -o

<테스트케이스>
컴파일 경로 : testcase.cpp msq.h -o

<make_DB 폴더 내용>

makeDB_admin.cpp & makeDB_client.cpp : 각각 DB_admin.dat와 DB_client.dat 파일을 생성
컴파일 경로 : makeDB_admin.cpp msq.h -o
컴파일 경로 : makeDB_client.cpp msq.h -o

readDB_admin.cpp & readDB_client.cpp : DB_admin.dat과 DB_client.dat의 내용을 문자로 출력
컴파일 경로 : readDB_admin.cpp msq.h -o
컴파일 경로 : readDB_client.cpp msq.h -o


server/DB_client.dat 내용
client1 asdf1234!@ dummy1 911005-1 122-141622-8235611 237000 
client2 asdf1234!@ dummy2 330624-1 022-283889-4629539 356000 
client3 asdf1234!@ dummy3 280310-1 033-825922-5337397 726000 
client4 asdf1234!@ dummy4 760820-2 011-592658-7371984 63000 
client5 asdf1234!@ dummy5 851010-2 133-522963-8849412 598000 

server/DB_admin.dat 내용
admin1 asdf1234!@
admin2 asdf1234!@
admin3 asdf1234!@
admin4 asdf1234!@