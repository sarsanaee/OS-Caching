#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <libmemcached/memcached.h>
#include <sys/time.h>
#include <assert.h>
#include <stdlib.h>
#include <math.h>
#include <signal.h>

static int allow_to_request = 1;


float nextTime(float rateParameter)
{
    float temp = -logf(1.0f - (float) random() / (RAND_MAX)) / rateParameter;
    return temp;
}

void sigintHandler(int signal)
{
    printf("CTRL pressed experiments ended\n");
    allow_to_request = 0;
}

void rand_str(char *dest, size_t length) {
    char charset[] = "0123456789"
                     "abcdefghijklmnopqrstuvwxyz"
                     "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    while (length-- > 0) {
        size_t index = (double) rand() / RAND_MAX * (sizeof charset - 1);
        *dest++ = charset[index];
    }
    *dest = '\0';
}

int main(int argc, char *argv[])
{
  memcached_server_st *servers = NULL;
  memcached_st *memc, *memc1;
  memcached_return rc;
  int now;
  int rate_limit;
  if(argc < 3)
  {
      fprintf(stderr, "./a.out rate ip \n");
      exit(1);
  }
  rate_limit = atoi(argv[1]);

  struct timeval time_before, time_next;

  char *key= "key";
  //char *value= "kasdfl;kjsaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
  uint32_t flags;
  size_t return_value_length;

  char * ip = argv[2];

  //Register Signal Handler
  signal(SIGINT, sigintHandler);

  //Random Number Generator

  // char nkey[] = { [41] = '\1' }; // make the last character non-zero so we can test based on it later
  // char nvalue[] = { [41] = '\1' };
  // rand_str(nkey, sizeof nkey - 1);
  // rand_str(nkey, sizeof nkey - 1);



  // assert(str[41] == '\0');      // test the correct insertion of string terminator
  // puts(str);

  long cLen = 1000000;
  char *value = (char *) malloc(cLen + 1);
  memset(value, 'a', cLen);
  value[cLen] = 0;
  memc = memcached_create(NULL);
  memc1 = memcached_create(NULL);
  servers= memcached_server_list_append(servers, ip, 11211, &rc);
  //rc = memcached_server_add_udp(servers, ip, 11211);
  //memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_USE_UDP, (uint64_t)1);

  rc = memcached_server_push(memc, servers);
  rc = memcached_server_push(memc1, servers);
  //memcached_behavior_set(memc, MEMCACHED_BEHAVIOR_USE_UDP, (uint64_t)1);

  rc = memcached_set(memc, key, strlen(key), value, strlen(value), (time_t)0, (uint32_t)0);

  if (rc == MEMCACHED_SUCCESS)
    fprintf(stderr,"Key stored successfully\n");
  else
    fprintf(stderr,"Couldn't store key: %s\n",memcached_strerror(memc, rc));
  char * res;


  //memcached_behavior_set(memc1, MEMCACHED_BEHAVIOR_USE_UDP, (uint64_t)1);

  while ( true)
  {
    now = nextTime(rate_limit) * 1000000;
    usleep(now);

    gettimeofday (&time_before, NULL);
    res = memcached_get(memc1, key, strlen(key), &return_value_length, &flags, &rc);
    gettimeofday(&time_next, NULL);
    free(res);
    if(rc != MEMCACHED_SUCCESS)
    {
      fprintf(stderr, "%s\n", memcached_strerror(memc1, rc));
    }

    if(allow_to_request == 0)
    {
	break;
    }


    int diff;
    diff = (time_next.tv_sec - time_before.tv_sec)*1000000 + (time_next.tv_usec - time_before.tv_usec);
    fprintf(stderr, "%d\n", diff);
  }

  memcached_free(memc);
  memcached_free(memc1);


  return 0;
}
