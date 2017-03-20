/* This file will contain your solution. Modify it as you wish. */
#include <types.h>
#include <synch.h>
#include <lib.h>    /* for panic */
#include "producerconsumer_driver.h"

/* Declare any variables you need here to keep track of and
   synchronise your bounded. A sample declaration of a buffer is shown
   below. You can change this if you choose another implementation. */

static struct pc_data buffer[BUFFER_SIZE];
volatile unsigned long int counter;
static struct lock *buffer_lock;
static struct cv *cv_full;
static struct cv *cv_empty;


/* consumer_receive() is called by a consumer to request more data. It
   should block on a sync primitive if no data is available in your
   buffer. */

struct pc_data consumer_receive(void)
{
        struct pc_data the_data;
        lock_acquire(buffer_lock);

        while (counter == 0) {
                cv_wait(cv_empty, buffer_lock);
        }

        the_data = buffer[counter - 1];
        counter--;

        cv_signal(cv_full, buffer_lock);
        lock_release(buffer_lock);
        return the_data;
}

/* procucer_send() is called by a producer to store data in your
   bounded buffer. */

void producer_send(struct pc_data item)
{
        lock_acquire(buffer_lock);

        while (counter == BUFFER_SIZE) {
                cv_wait(cv_full, buffer_lock);
        }

        buffer[counter] = item;
        counter++;

        cv_signal(cv_empty, buffer_lock);
        lock_release(buffer_lock);
}




/* Perform any initialisation (e.g. of global data) you need
   here. Note: You can panic if any allocation fails during setup */

void producerconsumer_startup(void)
{
        cv_full = cv_create("cv_full");
        if (cv_full == NULL) {
                panic("cv_full create failed");
        }
        cv_empty = cv_create("cv_empty");
        if (cv_empty == NULL) {
                panic("cv_empty create failed");
        }

        buffer_lock = lock_create("buffer_lock");
        if (buffer_lock == NULL) {
                panic("buffer lock create failed");
        }
        counter = 0;
}

/* Perform any clean-up you need here */
void producerconsumer_shutdown(void)
{
        lock_destroy(buffer_lock);
        cv_destroy(cv_full);
        cv_destroy(cv_empty);
}

