# gpr_mini_server_c

common/CJSON - 외부코드. 프로그램 환경 세팅을 json형태로 다루기 위해 사용
common/dir_control - 파일 제어 관련
common/gpr_param - 취득 데이터가 저장되는 파일 포맷형식
common/log - 외부코드. 프로그램이 실행되는 로그를 남기기 위해 사용
common/usb_control - 파일을 마운트한 usb에 복사

encoder/encoder_queue - 엔코더 발생 이벤트를 큐에 담에 놓고 후처리 하려다가 의미없어 보여 안함
encoder/encoder - 엔코더 및 아두이노 시리얼 통신 관련

gpr_socket/gpr_socket_acq - 소켓통신에서 취득 관련
gpr_socket/gpr_socket_anal - 소켓통신에서 분석 관련
gpr_socket/gpr_sockat_data - 소켓통신에서 데이터 처리 관련
gpr_soket/gpr_socket - 소켓통신 버퍼에서 이벤트 생성 관련
gpr_socket/wifi_selector - wifi 채널 변경 관련

NVA/GPR_TimingMeasurement - gpr mini 펌웨어 코드. 노벨다칩 세팅 관련
NVA/NVA_CON - gpr mini 펌웨어 코드. 노벨칩 세팅 관련
NVA/NVA_file - 노벨다칩 세팅 값을 파일에 저장
NVA/SPI - gpr mini 펌웨어 코드. 노벨다칩과 SPI 통신 관련
NVA/NVA6100 - gpr mini 펌웨어 코드. 노벨다칩 파라메타

main.c - 메인함수

CMakeList - c언어 cmake 빌드를 하기 위한 설정
acduino-code - led를 제어하는 아두이노 코드
host_restart.sh - wifi 채널변경을 쉘스크립트에서 소켓서버 재시작을 하려다가 정상동작 안하는 경우가 많아 현재는 사용안함
How to Program an AVR_Ardiono.. - 라즈베리에서 아두이노로 소스 업로드 하기 위한 가이드 파일
checksum 예제 - 소켓통신 checksum 만드는 과정 예제. Xbee 통신모듈을 참고하여 만듬

[c프로그램 빌드방법]
폴더: common, encoder, gpr_socket, NVA
파일: CMakeLists.txt, main.c
1. 위의 파일들을 라즈베리파이 경로로 복사
2. 해당 경로에서 ' sudo cmake CMkaeLists.txt ' 실행
3. ' sudo make ' 실행
4. ' cd Release ' 생성된 Release 폴더로 이동
5. ' sudo ./실행파일이름 ' 프로그램 실행

[윈도우에서 usb 연결하여 아두이노 업로드방법]
1. 윈도우에서 Arduino IDE 프로그램 실행
2. arduino_code.c 코드를 Arduino IDE 프로젝트에 복사
3. 스케치 -> 업로드

[라즈베리에서 아두이노 업로드방법]
1. 윈도우에서 Arduino IDE 프로그램 실행
2. arduino_code.c 코드를 Arduino IDE 프로젝트에 복사
3. 스케치 -> 컴파일된 바이너리 파일 내보내기
4. 아두이노 프로젝경로에서 ' 프로젝트이름.ino.with_bootloader.eightanaloginputs.hex ' 파일을 라즈베리로 복사
5. 라즈베리에서 ' How to Program an AVR_Arduino using... ' PDF 파일을 참고하여 아두이노로 업로드









