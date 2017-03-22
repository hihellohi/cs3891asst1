/* This file will contain your solution. Modify it as you wish. */
#include <types.h>
#include "producerconsumer_driver.h"
#include <synch.h>  /* for P(), V(), sem_* */
#include <lib.h>  /* kassert */

/* Declare any variables you need here to keep track of and
   synchronise your bounded. A sample declaration of a buffer is shown
   below. You can change this if you choose another implementation. */

static struct pc_data buffer[BUFFER_SIZE];
struct semaphore *full;
struct semaphore *empty;
struct lock *mutex;

int hi, lo;


/* consumer_receive() is called by a consumer to request more data. It
   should block on a sync primitive if no data is available in your
   buffer. */

struct pc_data consumer_receive(void)
{
        struct pc_data thedata;

		P(full);
		lock_acquire(mutex);

		thedata = buffer[lo++];

		lock_release(mutex);
		V(empty);


        return thedata;
}

/* procucer_send() is called by a producer to store data in your
   bounded buffer. */

void producer_send(struct pc_data item)
{
		P(empty);
		lock_acquire(mutex);

		buffer[hi++] = item;

		lock_release(mutex);
		V(full);
}




/* Perform any initialisation (e.g. of global data) you need
   here. Note: You can panic if any allocation fails during setup */

void producerconsumer_startup(void)
{
	full = sem_create("full", 0);
	empty = sem_create("empty", BUFFER_SIZE);
	mutex = lock_create("mutex");
	KASSERT(full != NULL && empty != NULL && mutex != NULL);
	hi = lo = 0;
}

/* Perform any clean-up you need here */
void producerconsumer_shutdown(void)
{
	sem_destroy(full);
	sem_destroy(empty);
	lock_destroy(mutex);
}

