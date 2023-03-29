#ifndef gpr_socket_data__h
#define gpr_socket_data__h

//소켓 버퍼를 받는 구조
struct TcpData
{
    int index; //받은 버퍼를 처리하고 있는 현재위치
    int length; //받은 버퍼의 길이
    int checkSum; //무결성 검증을 위해 받은 버퍼를 합산
    char *data_buffer; //받은 버퍼를 저장
};

extern struct TcpData tcpData;

int bytesToInt(char *buffer, int size);
void intToBytes(int integer, char *buffer, int size);
void convertEvent(char buffer[], int buffer_size);
void eliminate_json(char *str);
char *arrayCodeToStr(char *arrayCode);
void setHeaderFromJson(char *bytes);
void setAcqInfoFromJson(char *bytes);
char *jsonForsendSavePath();

#endif