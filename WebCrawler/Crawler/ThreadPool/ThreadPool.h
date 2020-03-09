
#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_



#include <stdio.h> 		// from www.mario-konrad.ch
#include <pthread.h>
#include <unistd.h>
#include "List.h"






static pthread_mutex_t mtx2;
static pthread_mutex_t mtx;
static pthread_cond_t cond_nonempty;
static pthread_cond_t cond_nonfull;

inline bool AllNotWorking(bool* i_isworking,unsigned int threads_num)
{
	unsigned int count=0;
	for(unsigned int i=0;i<threads_num;i++)
	{
		if(i_isworking[i]==false)
		{
			//printf("NOT working");fflush(stdout);
			count++;
		}
	}
	return (count==threads_num);

}



inline void place(List* p, char* data) {
	pthread_mutex_lock(&mtx);

	p->Insert(data);

	pthread_mutex_unlock(&mtx);
}


inline char* obtain(List* p,bool* &i_isworking,unsigned int threads_num,int i) {
	char* data=NULL;
	  pthread_mutex_lock(&mtx);
	while (p->Is_Empty()) {
		i_isworking[i]=false;

		pthread_cond_wait(&cond_nonempty, &mtx);
		}
		i_isworking[i]=true;
		data=p->Remove_HeadData();

		pthread_mutex_unlock(&mtx);
	return data;
}



#endif
