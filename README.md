Capstone_encryted_control
=============
2025 Capstone project repository

Description
====
- 현재 한 루프에 30ms 이내 (프린트 문을 빼도 비슷한...)
- 한번씩 통신 오류 발생
- com_utils : 파일 읽고 쓰기 관련 함수
- backup : 백업용 


Requirment
=============
Go 설치하기

인터넷 연결 > ifconfig 로 ip 확인
or
127.0.0.1 로 설정해서 한 컴퓨터로 통신 시뮬레이션 돌려보기
(이렇게 했을 시에는 통신 오류 X)

Preliminary
===
1. RLWE
2. ARX form controller

Usage
=============



```
git clone https://github.com/lsw23101/Enc_control_RLWE
```



```
sudo apt update
sudo apt install g++ libgmp-dev libmpfr-dev libboost-all-dev cmake python3-dev


pip install fpylll 
// 설치 까다로움
```




plant와 controller 코드에서 ip 설정

<terminal 1>
```
cd ~/Rasberry
```

```
go run plant.go
```

<terminal 2>
```
cd ~/Computer
```

```
go run controller.go
```

Todo
====

통신 환경에 따라 루프 당 걸리는 시간 변동이 심함 

중간에 통신 끊기는 상황 예외 처리

코드 정리



N이 작을때의 원격 컨트롤러 구현해서 비밀 키 빼내는 부분 제작



