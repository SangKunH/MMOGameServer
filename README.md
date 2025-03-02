
C++ 네트워크 프로래밍, 멀티스레드 프로그래밍 프로젝트

스킬셋 : C++20, IOCP, Protobuf, nlohmann(json), SQL-Server

github : https://github.com/WillySJHan/MMOGameServer

시현 영상 : https://youtu.be/qx00uF7cN10

-------------------------------
![Image](https://github.com/user-attachments/assets/5ed558c9-01f1-43a8-95f8-291f7c34af13)
--------------------------------


1. 서버구조

--------------------------
![Image](https://github.com/user-attachments/assets/b66f7a65-6bcb-41a4-83ac-f2bf31d908fa)
----------------------------------

Task의 예약을 관리하는 TaskSchedular(우선순위 큐) 1개

TaskQueue의 Queue 1개

TaskQueue는 각 Room에 한 개씩 2개

DataBase와 Transaction 하는 TaskQueue 1개 

총 3개


- Worker Thread
  
----------------------------------------------
![Image](https://github.com/user-attachments/assets/e46746de-3613-4d66-a80e-084f4376ae8b)
-----------------------------------------------------

Room의 GameLogic은 싱글 스레드로 구현 WorkerThread들은 TaskQueue에 Task를 Push 하고

만약 해당 TaskQueue를 점유하고 있는 WorkerThread가 없다면 일정 시간 동안 해당 TaskQueue의 Task를 

처리하고 TaskQueue의 TaskQueue에 Push

-> 모든 스레드 들은 IOCP에서 완료 통지를 받은 후 작업 완료

-> TaskSchedular에 예약된 Task가 있다면 TaskQueue에 Push

-> TaskQueue의 Queue에 TaskQueue가 있다면 Pop 후 TaskQueue에 있는 Task 수행

-> TaskQueue의 Task를 수행 도중 endTime이 지난다면 다시 TaskQueue의 Queue에 TaskQueue를 Push후 종료

* endTime을 도입한 이유 (Thread Local Storage 변수)
  
만약 TaskQueue가 Thread의 수보다 많아진다면 하나의 TaskQueue에 너무 많은 작업이 몰리면 다른 TaskQueue에 있은 Task를 수행하지 못하는 문제점 발생하기 때문입니다.


- Task
-----------------------
![Image](https://github.com/user-attachments/assets/22f4bc02-c24c-4b57-9b6c-f481b657f8cf)
--------------------------------

게임 로직을 수행하는 함수에서 mutex의 lock을 사용하게 되면 해당 게임 로직이 끝나기 전에는 다른 스레드들이 대기해야 하기 때문에 Task를 도입하였습니다.

shared_ptr와 람다를 이용하여 Task 객체를 생성합니다.

Monster, Projectile 같은 객체의 Update를 Task 객체로 생성 후 재귀 함수처럼 TaskSchedular에 예약하여 주기적으로 호출되게 하였습니다.


- Scatter Gather

----------------------------------------------------------
![Image](https://github.com/user-attachments/assets/a68cdada-35d4-47eb-a8c9-45fd8e8b743f)
----------------------------

WSASend()의 빈번한 호출을 막기 위해 

이미 WSASend()가 등록되어 있다면 Session에서 Send를 호출할 때마다 WSASend()를 호출하지 않고 ProcessSend()까지 다 끝난 후 그동안 모인 SendBuffer를 한 번에 WSASend() 합니다.


- ProtocolManager.h 소스코드 자동화

----------------------------------------------------------------
![Image](https://github.com/user-attachments/assets/24098baf-2988-4d95-9e76-8cb8bc082382)
--------------------------------------------------------------------

파이썬의 jinja2, pyinstaller 패키지를 이용하여 Protobuf의 .proto 파일을 읽어 동적으로 ProtocolManager.h의 소스코드를 자동으로 생성해 주는 .exe 파일을 생성합니다.


- 게임데이터, 설정 파일 nlohmann json

------------------------------------------------------------------------
![Image](https://github.com/user-attachments/assets/7bf375a2-8a39-4c37-b181-d6cc0fb806b4)
------------------------------------------------------------------------

서버가 실행될 때 nlohman json 라이브러리를 이용하여 Data.json 파일을 파싱 하여 unordered_map으로 불러옵니다.


2.게임로직


- 이동 동기화
  
클라이언트에서 이동키를 누르면 셀단위(1칸) 선 이동 후 서버에게 전송

서버는 주변 클라이언트에게 Braodcasting


- 피격처리
  
서버에서 총알을 움직이고 클라이언트에게 Broadcasting 만약 총알이 더 이상 앞으로 이동 불가능 한데 만약 

앞을 막은 객체가 플레이어이거나 몬스터이면 피격


- 시야처리

Broadcasting 부하를 줄이기위해서 시야를 도입

-------------------------------------------------------
![Image](https://github.com/user-attachments/assets/d08e4913-f462-4d16-8ceb-d158af08ca53)
---------------------------------------------------

Room를 일정 크기의 Area들로 나누고 VisualField는 자신 속한 Area들 안의 객체들만 거리를 계산하여 

VisualField 안의 객체들과 정보 송수신하게 하였습니다.

0.2초에 한 번씩 VisualField를 최신화하여 despawn, spawn, 객체 이동 최신화하였습니다.


- A* 몬스터 길 찾기
  
유한 상태 기계와 A*를 이용하여 몬스터 AI, 몬스터 길 찾기 구현하였습니다.

closeList와 openList, 경로 계산을 위한 parent를 처음에는 2차원 vector을 이용하였으나 길 찾기 함수를 자주 호출할 때마다 초기화할 때 부하가 크기 때문에 unordered_map과 unordered_set으로 변경하여 성능을 향상시켰습니다.


- 인벤토리, 아이템 장착, 아이템 드랍
  
게임에 입장하게 되면 데이터베이스에서 해당 플레이어의 아이디에 맞는 인벤토리 데이터를 가져옵니다.

그 후 클라이언트에게도 해당 인벤토리 정보를 전송합니다.

클라이언트에서 아이템을 장착하게 되면 서버에서 장착이 가능하면 서버에서 먼저 장착을 한 후 데이터베이스에 적용하고 클라이언트에게 장착해도 좋다는 패킷을 전송합니다.

몬스터가 죽게 되면 몬스터를 죽인 플레이어에게 아이템을 랜덤으로 지급합니다.

데이터베이스에도 적용합니다.


- 더미 클라이언트를 통한 부하 테스트
  
더미 클라이언트도 서버와 같은 IOCP를 활용하여 구현하였습니다.

각 Room에 몬스터 500마리를 생성하고 더미 클라이언트 1000개를 연결하였습니다.

총 1000마리의 몬스터와 2000개의 더미 클라이언트 연결합니다.


3.유틸리티


- 인코딩 변환

--------------------------------------
![Image](https://github.com/user-attachments/assets/eee98705-d72d-428d-a739-75da08acc8ed)
--------------------------------------

Protobuf의 string은 utf8 인코딩 문자열을 받기 때문에 제가 사용한 wsting(utf16) 문자열 인코딩을 

string(utf8)으로 변환하기 위해 Utility 함수를 제작하였습니다.


- 랜덤 함수

----------------------------
![Image](https://github.com/user-attachments/assets/f834da98-27d1-47b7-83d2-cb283384ed34)
---------------------------------------

인자로 받는 범위 변수의 정수형, 실수형에 따라 랜덤 값을 return 하게 만들었습니다.
