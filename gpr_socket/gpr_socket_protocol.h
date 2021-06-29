#ifndef gpr_socket_param__h
#define gpr_socket_param__h

enum SocketProtocol
{
    //최초 연결
    CONNECTION_NTF = 0x01,
    //서버에서 연결을 거부할때
    DISCONNECTION_NTF = 0x02,
    //서버로 보내는 Header 정보
    HEADER_INFO_FTN = 0x03,
    //각종 상태 체크(배터리, 안테나 등)
    SERVER_INFO_NTF = 0x04,
    //취득 정보 받음
    ACQ_INFO_FTN = 0x05,
    //저장 경로 보냄
    ACQ_SAVE_PATH_NTF = 0x06,
    //레이저 on 요청
    ACQ_LASER_ON_FTN = 0x07,

    //취득 시작
    ACQ_START_FTN = 0x11,
    ACQ_START_NTF = 0x12,
    ACQ_DATA_NTF = 0x13,
    ACQ_STOP_FTN = 0x14,
    ACQ_STOP_NTF = 0x15,
    ACQ_SAVE_FTN = 0x16,
    ACQ_SAVE_NTF = 0x17,
    ACQ_NON_SAVE_FTN = 0x18,
    ACQ_REFRESH_FTN = 0x19,
    ACQ_REFRESH_NTF = 0x1A,
    ACQ_BACK_DATA_NTF = 0x1B,

    //NVA 세팅값 요청
    NVA_REQUEST_FTN = 0x25,
    //NVA 세팅값 보냄
    NVA_RESPONSE_NTF = 0x26,
    //NVA 세팅값 수정
    NVA_MODIFY_FTN = 0x27,
    //NVA 세팅값 완료
    NVA_COMPLETE_NTF = 0x28,

    //에러 사항
    ACQ_ABNORMAL_QUIT = 0x31,

    //분석 ㅠㅠ
    ANA_ROOT_DIR_FTN = 0x51,
    ANA_ROOT_DIR_NTF = 0x52,
    ANA_READ_DIR_FTN = 0x53,
    ANA_READ_DIR_NTF = 0x54,
    //ANA_READ_DIR_FTN랑 같음 ㅠㅠ 처리만 다르게..
    ANA_CHECK_DIR_FTN = 0x55,
    ANA_CHECK_DIR_NTF = 0x56,
    ANA_UNCHECK_DIR_FTN = 0x57,
    ANA_UNCHECK_DIR_NTF = 0x58,

    ANA_DISK_SIZE_FTN = 0x61,
    ANA_DISK_SIZE_NTF = 0x62,

    ANA_DELETE_FILE_FTN = 0x71,
    ANA_DELETE_FOLDER_FTN = 0x72,
    ANA_DELETE_FILE_NTF = 0x73,
    ANA_DELETE_FOLDER_NTF = 0x74,

    ANA_USB_INFO_FTN = 0x81,
    ANA_USB_INFO_NTF = 0x82,
    ANA_USB_INFO_FAILED_NTF = 0x83,
    ANA_USB_COPY_FTN = 0x84,
    ANA_USB_COPY_NTF = 0x85,
    ANA_USB_COPYING_NAME_NTF = 0x86,
    ANA_USB_COPY_DONE_NTF = 0x87,
    ANA_USB_COPY_FAILED_NAME_NTF = 0x88,

    //gpr file data만 불러옴
    ANA_LOAD_FILE_FTN = 0x91,
    ANA_LOAD_FILE_NTF = 0x92,
    //위와 동일한데 tcp로 gpr data를 받으면 config 설정 파일을 요청함.
    ANA_LOAD_FILE_WITH_CONFIG_FTN = 0x93,
    ANA_LOAD_FILE_WITH_CONFIG_NTF = 0x94,
    ANA_FILE_SIZE_NTF = 0x95,
    ANA_FILE_DATA_NTF = 0x96,
    //config 파일
    ANA_LOAD_CONFIG_FILE_FTN = 0x97,
    ANA_LOAD_CONFIG_FILE_NTF = 0x98,
    //config 파일 저장
    ANA_SAVE_CONFIG_FILE_FTN = 0x99,
    ANA_SAVE_CONFIG_FILE_NTF = 0x9A,
    //topview를 볼 때
    ANA_LOAD_TOP_VIEW_FNT = 0x9B,
    ANA_LOAD_TOP_VIEW_NTF = 0x9C,
} socketProtocol;

#endif