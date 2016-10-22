#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <ctype.h>
#include "stat_server.h"
#include "stat.h"

#define STDOUT 1
#define STDERR 2
#define ERROR_USAGE "Usage: stat_server -k [key]"
#define ERROR_SHMGET "shmget failed"
#define MAX_CLIENTS 16

int num_clients = 0;

// Prints whether the given string is a number
int is_number(char *num) {
  int i = 0;
  while (i < strlen(num)) {
    if (!isdigit(num[i])) {
      return 0;
    }
    i++;
  }
  return 1;
}

// Print the number of characters in the given integer
int num_chars_in_int(int integer) {
  int tens, num_chars = 1;
  if (integer < 0) {
    num_chars = 1;
    tens = -1;
    while (tens > integer) {
      tens *= 10;
      num_chars++;
    }
  } else {
    tens = 1;
    while (tens < integer) {
      tens *= 10;
      num_chars++;
    }
  }
  return num_chars;
}


int main(int argc, char *argv[]) {
  // Check for usage errors
  if (argc != 3 || strcmp(argv[1], "-k") != 0 || !is_number(argv[2])) {
    write(STDERR, ERROR_USAGE, strlen(ERROR_USAGE));
    exit(1);
  }

  // Declaring all variables required
  stats_t stat;
  key_t key;
  int i = 0;
  long pagesize = getpagesize();
  int seg_id = shmget(key, pagesize, IPC_CREAT);
  char *line = (char*) malloc(sizeof(stats_t));
  stat_t *ptr, *rd_ptr;
  // Get pointer to shared memory
  ptr = (stat_t*) shmat(seg_id, NULL, 0);

  sem_t sem;
  sem_init(&sem, 0, 1);
  // Infinite loop to read data
  while (1) {
    sem_wait(&sem);
    // Print all the client statistics
    for (j = 0, rd_ptr = ptr; j < num_clients; j++, rd_ptr++) {
      char *line = (char*) malloc(sizeof(stats_t) - sizeof(int) + 6 +
                                  strlen(rd_ptr->name) + num_chars_in_int(i));
      sprintf(line, "%d %d %s %d %.2f %d\n", i, rd_ptr->pid, rd_ptr->name,
              rd_ptr->counter, rd_ptr->cpu_secs, rd_ptr->priority);
      write(STDOUT, line, strlen(line));
      free(line);
    }
    sem_post(&sem);
    i++;
    sleep(1);
  }
}
