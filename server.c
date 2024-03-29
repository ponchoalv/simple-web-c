#include "dbg.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthreadpool.h>
#include <signal.h>

#define PORT 9090
#define BUFFER_SIZE 104857600
#define THREAD_COUNT 6

const char *get_file_extension(const char *file_name) {
  const char *dot = strrchr(file_name, '.');
  if (!dot || dot == file_name) {
    return "";
  }
  return dot + 1;
}

const char *get_mime_type(const char *file_ext) {
  if (strcasecmp(file_ext, "html") == 0 || strcasecmp(file_ext, "htm") == 0) {
    return "text/html";
  } else if (strcasecmp(file_ext, "txt") == 0) {
    return "text/plain";
  } else if (strcasecmp(file_ext, "jpg") == 0 ||
             strcasecmp(file_ext, "jpeg") == 0) {
    return "image/jpeg";
  } else if (strcasecmp(file_ext, "png") == 0) {
    return "image/png";
  } else {
    return "application/octet-stream";
  }
}

const char *get_first_line(char *buffer) {
  char *cur_line = buffer;
  char *next_line = strchr(cur_line, '\n');
  if (next_line)
    *next_line = '\0';

  return cur_line;
}

bool case_insensitive_compare(const char *s1, const char *s2) {
  while (*s1 && *s2) {
    if (tolower((unsigned char)*s1) != tolower((unsigned char)*s2)) {
      return false;
    }
    s1++;
    s2++;
  }
  return *s1 == *s2;
}

char *get_file_case_insensitive(const char *file_name) {
  DIR *dir = opendir(".");
  if (dir == NULL) {
    perror("opendir");
    return NULL;
  }

  struct dirent *entry;
  char *found_file_name = NULL;
  while ((entry = readdir(dir)) != NULL) {
    if (case_insensitive_compare(entry->d_name, file_name)) {
      found_file_name = entry->d_name;
      break;
    }
  }

  closedir(dir);
  return found_file_name;
}

char *url_decode(const char *src) {
  size_t src_len = strlen(src);
  char *decoded = malloc(src_len + 1);
  size_t decoded_len = 0;

  // decode %2x to hex
  for (size_t i = 0; i < src_len; i++) {
    if (src[i] == '%' && i + 2 < src_len) {
      int hex_val;
      sscanf(src + i + 1, "%2x", &hex_val);
      decoded[decoded_len++] = hex_val;
      i += 2;
    } else {
      decoded[decoded_len++] = src[i];
    }
  }

  decoded[decoded_len] = '\0';
  return decoded;
}

void build_http_response(const char *file_name, const char *file_ext,
                         char *response, size_t *response_len) {
  // header
  const char *mime_type = get_mime_type(file_ext);
  char *header = (char *)malloc(BUFFER_SIZE * sizeof(char));
  snprintf(header, BUFFER_SIZE,
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: %s\r\n"
           "\r\n",
           mime_type);

  int file_fd = open(file_name, O_RDONLY);
  if (file_fd == -1) {
    snprintf(response, BUFFER_SIZE,
             "HTTP/1.1 404 Not Found\r\n"
             "Content-Type: text/plain\r\n"
             "\r\n"
             "404 Not Found");
    *response_len = strlen(response);
    free(header);
    return;
  }

  *response_len = 0;
  memcpy(response, header, strlen(header));
  *response_len += strlen(header);

  ssize_t bytes_read;
  while ((bytes_read = read(file_fd, response + *response_len,
                            BUFFER_SIZE - *response_len)) > 0) {
    *response_len += bytes_read;
  }
  free(header);
  close(file_fd);
}

void handle_client(void *arg, size_t index) {
  int client_fd = *((int *)arg);
  char *buffer = (char *)malloc(BUFFER_SIZE * sizeof(char));

  ssize_t bytes_received = recv(client_fd, buffer, BUFFER_SIZE, 0);
  if (bytes_received > 0) {
    // check if request is GET
    regex_t regex;
    regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
    regmatch_t matches[2];

    log_info("Received %s\n", get_first_line(buffer));

    if (regexec(&regex, buffer, 2, matches, 0) == 0) {
      buffer[matches[1].rm_eo] = '\0';
      const char *url_encoded_file_name = buffer + matches[1].rm_so;
      char *file_name = url_decode(url_encoded_file_name);

      char file_ext[32];
      strcpy(file_ext, get_file_extension(file_name));

      char *response = (char *)malloc(BUFFER_SIZE * 2 * sizeof(char));
      size_t response_len;
      build_http_response(file_name, file_ext, response, &response_len);

      send(client_fd, response, response_len, 0);

      free(response);
      free(file_name);
    }
    regfree(&regex);
  }
  close(client_fd);
  free(arg);
  free(buffer);

  return;
}

int main(int argc, char *argv[]) {
  int server_fd;
  struct sockaddr_in server_addr;
  pthreadpool_t thread_pool = pthreadpool_create(THREAD_COUNT);

  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("bind failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, 6) < 0) {
    perror("listen failed");
    close(server_fd);
    exit(EXIT_FAILURE);
  }

  log_info("Server listening on port %d\n", PORT);

  while (1) {
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int *client_fd = malloc(sizeof(int));

    if ((*client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                             &client_addr_len)) < 0) {
      perror("accept failed");
      continue;
    }

    pthreadpool_parallelize_1d(thread_pool, handle_client, client_fd, 1, 0);
  }

  pthreadpool_destroy(thread_pool);
  close(server_fd);
  return 0;
}
