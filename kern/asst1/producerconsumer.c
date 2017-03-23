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
struct lock *head, *tail;

int hi, lo;


/* consumer_receive() is called by a consumer to request more data. It
   should block on a sync primitive if no data is available in your
   buffer. */

struct pc_data consumer_receive(void)
{
        struct pc_data thedata;

		P(full);
		lock_acquire(tail);
		//the locks head and tail will be simultaneously be aquired iff hi != lo

		thedata = buffer[lo++];
		lo %= BUFFER_SIZE;

		lock_release(tail);
		V(empty);


        return thedata;
}

/* procucer_send() is called by a producer to store data in your
   bounded buffer. */

void producer_send(struct pc_data item)
{
		P(empty);
		lock_acquire(head);

		buffer[hi++] = item;
		hi %= BUFFER_SIZE;

		lock_release(head);
		V(full);
}




/* Perform any initialisation (e.g. of global data) you need
   here. Note: You can panic if any allocation fails during setup */

void producerconsumer_startup(void)
{
	full = sem_create("full", 0);
	empty = sem_create("empty", BUFFER_SIZE);
	head = lock_create("head");
	tail = lock_create("tail");
	KASSERT(full != NULL && empty != NULL && head != NULL && tail != NULL);
	hi = lo = 0;
}

/* Perform any clean-up you need here */
void producerconsumer_shutdown(void)
{
	sem_destroy(full);
	sem_destroy(empty);
	lock_destroy(head);
	lock_destroy(tail);
}

