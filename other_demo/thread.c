
#include<stdio.h>  
#include<sys/types.h>  
#include<stdlib.h>  
#include<unistd.h>  
#include<pthread.h>  
/********************************************************
gcc build: gcc  test.c -g -o test  -lpthread
gdb debug: gdb ./test
set breakpoint : break 46
run : run
next: next
into function: step
next: next
next: next
exit function: finish
show variable: print i
set variable: set variable i=5
show variable: print i
next: next
help: help all
exit gdb: q
********************************************************/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  
pthread_cond_t  cond  = PTHREAD_COND_INITIALIZER;  
  
int count = 0;
pthread_t threadInc, threadDec;

void *decrement(void *arg) {  
    printf("in derement.\n");  
    pthread_mutex_lock(&mutex);  
    if (count == 0)  {
        printf(" pthread_cond_wait.\n");
        pthread_cond_wait(&cond, &mutex);  
        printf("over pthread_cond_wait.\n");
    }

    count--;  
    printf("----decrement:%d.\n", count);  
    printf("out decrement.\n");  
    pthread_mutex_unlock(&mutex); 
    printf("pthread_exit start.\n"); 

    pthread_exit(NULL);
    printf("pthread_exit.\n"); 
    return NULL;  
}  
  
void *increment(void *arg) {  
    printf("in increment.\n");  
    pthread_mutex_lock(&mutex);  
    count++;  
    printf("----increment:%d.\n", count);  
    if (count != 0)  
        pthread_cond_signal(&cond);  
 
    printf("out increment.\n");  
    pthread_mutex_unlock(&mutex);  
    return NULL;  
}  

int main(int argc, char *argv[]) {  
    
    pthread_mutex_lock(&mutex);  
    pthread_cond_signal(&cond);  
    printf("pthread_cond_signal cond.\n");  
    pthread_mutex_unlock(&mutex);  

    pthread_create(&threadDec, NULL, (void*)decrement, NULL);  
    sleep(2);  
    pthread_create(&threadInc, NULL, (void*)increment, NULL);  
    sleep(5);  

	printf("pthread_join start.\n");
    pthread_join(threadDec, NULL);

	printf("pthread_join end.\n"); 
    pthread_join(threadInc, NULL);  
    pthread_mutex_destroy(&mutex);  
    pthread_cond_destroy(&cond);  
    return 0;  
}  
