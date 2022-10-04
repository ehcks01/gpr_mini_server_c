#ifndef gpr_socket_data__h
#define gpr_socket_data__h

struct TcpData
{
    int index;
    int length;
    int checkSum;
    char *data_buffer;
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