#ifndef NVA_file__h
#define NVA_file__h

bool getNVAPath(char path[]);
char *getNVAJson();
void saveNVASetting();
char *readNVAFile();
void setNVASetting(char *bytes);
void loadNVASetting();

#endif

