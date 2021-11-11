#ifndef NVA_file__h
#define NVA_file__h

bool initNVAPath();
char *getNVAJson();
void saveNVASetting();
char *readNVAFile();
void setNVASetting(char *bytes);
void loadNVASetting();

extern char strNVAPath[100];
#endif

