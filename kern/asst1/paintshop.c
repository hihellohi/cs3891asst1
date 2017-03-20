#include <types.h>
#include <lib.h>
#include <synch.h>
#include <test.h>
#include <thread.h>

#include "paintshop.h"
#include "paintshop_driver.h"


/*
 * **********************************************************************
 * YOU ARE FREE TO CHANGE THIS FILE BELOW THIS POINT AS YOU SEE FIT
 *
 */

/* Declare any globals you need here (e.g. locks, etc...) */

static struct paintorder *buffer[NCUSTOMERS];
volatile unsigned long int counter;
static struct lock *tint_locks[NCOLOURS];
static struct lock *buffer_lock;
static struct lock *mix_lock;
static struct cv *cv_empty;

/*
 * **********************************************************************
 * FUNCTIONS EXECUTED BY CUSTOMER THREADS
 * **********************************************************************
 */

/*
 * order_paint()
 *
 * Takes one argument referring to the order to be filled. The
 * function makes the order available to staff threads and then blocks
 * until the staff have filled the can with the appropriately tinted
 * paint.
 */

void order_paint(struct paintorder *order)
{
        order->finished = sem_create("finished", 0);
        lock_acquire(buffer_lock);

        buffer[counter] = order;
        counter++;

        cv_signal(cv_empty, buffer_lock);

        lock_release(buffer_lock);

        P(order->finished);
        sem_destroy(order->finished);
}



/*
 * **********************************************************************
 * FUNCTIONS EXECUTED BY PAINT SHOP STAFF THREADS
 * **********************************************************************
 */

/*
 * take_order()
 *
 * This function waits for a new order to be submitted by
 * customers. When submitted, it returns a pointer to the order.
 *
 */

struct paintorder *take_order(void)
{
        struct paintorder *order = NULL;

        lock_acquire(buffer_lock);

        while (counter == 0) {
                cv_wait(cv_empty, buffer_lock);
        }

        order = buffer[counter - 1];
        counter--;

        lock_release(buffer_lock);

        return order;
}


/*
 * fill_order()
 *
 * This function takes an order provided by take_order and fills the
 * order using the mix() function to tint the paint.
 *
 * NOTE: IT NEEDS TO ENSURE THAT MIX HAS EXCLUSIVE ACCESS TO THE
 * REQUIRED TINTS (AND, IDEALLY, ONLY THE TINTS) IT NEEDS TO USE TO
 * FILL THE ORDER.
 */

void fill_order(struct paintorder *order)
{

        /* add any sync primitives you need to ensure mutual exclusion
           holds as described */
        lock_acquire(mix_lock);
        int i;
        for (i = 0; i < PAINT_COMPLEXITY; i++) {
            int tint = order->requested_tints[i];
            if (tint > NCOLOURS)
                    panic("Unknown colour");
            if (tint > 0)
                lock_acquire(tint_locks[tint - 1]);
        }
        lock_release(mix_lock);

        /* the call to mix must remain */
        mix(order);

        for (i = 0; i < PAINT_COMPLEXITY; i++) {
            int tint = order->requested_tints[i];
            if (tint > 0)
                lock_release(tint_locks[tint - 1]);
        }
}


/*
 * serve_order()
 *
 * Takes a filled order and makes it available to (unblocks) the
 * waiting customer.
 */

void serve_order(struct paintorder *order)
{
        V(order->finished);
}



/*
 * **********************************************************************
 * INITIALISATION AND CLEANUP FUNCTIONS
 * **********************************************************************
 */


/*
 * paintshop_open()
 *
 * Perform any initialisation you need prior to opening the paint shop
 * to staff and customers. Typically, allocation and initialisation of
 * synch primitive and variable.
 */

void paintshop_open(void)
{
        cv_empty = cv_create("cv_empty");
        if (cv_empty == NULL) {
                panic("cv_empty create failed");
        }

        int i;
        for (i = 0; i < NCOLOURS; i++) {
            struct lock *tint_lock = lock_create("tint_lock");
            if (tint_lock == NULL) {
                    panic("tint_lock create failed");
            }
            tint_locks[i] = tint_lock;
        }

        buffer_lock = lock_create("buffer_lock");
        if (buffer_lock == NULL) {
                panic("buffer_lock create failed");
        }

        mix_lock = lock_create("mix_lock");
        if (mix_lock == NULL) {
                panic("mix_lock create failed");
        }
        counter = 0;
}

/*
 * paintshop_close()
 *
 * Perform any cleanup after the paint shop has closed and everybody
 * has gone home.
 */

void paintshop_close(void)
{
        cv_destroy(cv_empty);
        int i;
        for (i = 0; i < NCOLOURS; i++) {
            lock_destroy(tint_locks[i]);
        }
        lock_destroy(buffer_lock);
        lock_destroy(mix_lock);
}

