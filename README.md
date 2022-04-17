# Smart-Trash-Arduino

# 설계 내용
일반적인 분리수거장에 스마트 iot 기술을 접목시켜 분리수거의 정확도를 높이고 에너지 절감효과를 보여줌으로써 재활용 하는 것을 격려 할 수 있다. 
수집된 쓰레기의 양이 데이터로 누적되어 서버에 기록되고 관리자는 스마트폰으로 쉽게 분리수거장이 상태정보를 확인 할 수 있다.
분리수거장 자체를 관리할 수 있는 main_board는 방범기능과 여러 정보를 api를 사용하여 관리자에게 제공함으로써  더욱 스마트한 재활용 시스템을 구현하였다.


## sensor Board(smart trash can) HW


a. IO cube

	D0:버튼
	D2:LED
	A0:조도센서
	A1:가변저항(중량계로 치환)
b. 부저

c. 터치센서(L,R)

d. led matrix

e. RGB led

f. GPIO 15번 핀

g. wifi module


## main Board(분리수거장 관리 보드) HW

a. real cube

	- 온습도, 압력 센서
	- 마이크 센서
	
b. 부저

c. led matrix

d. RGB led

e. GPIO 15번 핀

f. Bluetooth module

g. Serial port



# 기능 내용
## sensor Board(smart trash can) SW


쓰레기선택 모드일 때 왼쪽을 누르면 P->G->C->P 순으로 상태 변화

쓰레기선택 모드일 때 오른쪽을 누르면 P->C->G->P 순으로 상태 변화




## main Board(분리수거장 관리 보드) SW

1. 로그인 기능 (로그인 시간 기록)
2. api를 이용한 재활용 발생량 정보 취득
3. api를 이용한 폐기물 발생량 정보 취득
4. api를 이용한 주변 악취 정보 취득
5. api를 이용한 미세먼지 농도 정보 취득
6. api를 이용한 기온 정보 취득
7. 마이크 센서를 사용한 방범 기능
8. 온습도 센서를 사용한 불쾌지수 출력
9. 로그아웃 기능(close)

-API 3개 사용
