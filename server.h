#ifndef SERVER_H
#define SERVER_H

// Funciones que se utilizan en server.c y que se necesitan en test_server.c
void handle_client(void* arg);
char* get_file_case_insensitive(const char* filename);
char* url_decode(const char* str);

#endif // SERVER_H