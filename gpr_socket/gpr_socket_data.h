#ifndef gpr_socket_data__h
#define gpr_socket_data__h

struct TcpData
{
    unsigned int event_length;   //total buffer에서 자를 이벤트의 길이
    unsigned int total_length;   //total_buffer의 길이
    char *total_buffer;          //소켓으로 받고 있는 버퍼의 합
    unsigned int event_list_cnt; //리스트에 있는 이벤트 개수
    char **event_list;           //이벤트가 들어있는 리스트
    int *event_length_list;      //리스트에 들어 있는 각 이벤트의 길이
};

extern struct TcpData tcpData;

int bytesToInt(char *buffer, int size);
void intToBytes(int integer, char *buffer, int size);
void convertEvent(char buffer[], int buffer_size);
void eliminate_json(char *str);
char *arrayCodeToStr(char *arrayCode);
void setHeaderFromJson(char *bytes);
void setAcqInfoFromJson(char *bytes);
char *jsonOfsendSavePath();

#endif