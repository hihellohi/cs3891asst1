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

struct paintorder *buffer[NCUSTOMERS];
struct semaphore *empty;
struct lock *ins, *rem, *mixlocks[NCOLOURS];
int hi, lo;

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
		order->sem = sem_create("order", 0);
		KASSERT(order->sem != NULL);

		lock_acquire(ins);

		buffer[hi++] = order;
		hi %= NCUSTOMERS;
		
		V(empty);
		lock_release(ins);

		P(order->sem);
		sem_destroy(order->sem);
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
        struct paintorder *ret = NULL;

		lock_acquire(rem);
		P(empty);
		
		ret = buffer[lo++];
		lo %= NCUSTOMERS;

		lock_release(rem);

        return ret;
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
		char requested[NCOLOURS];
		for(int i = 0; i < NCOLOURS; i++){
			requested[i] = 0;
		}

		unsigned int cur;
		for(int i = 0; i < PAINT_COMPLEXITY; i++){
			if((cur = order->requested_tints[i]) && cur <= NCOLOURS){
				requested[cur - 1] = 1;
			}
		}

		for(int i = 0; i < NCOLOURS; i++){
			if(requested[i]){
				lock_acquire(mixlocks[i]);
			}
		}

        mix(order);

		for(int i = 0; i < NCOLOURS; i++){
			if(requested[i]){
				lock_release(mixlocks[i]);
			}
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
		V(order->sem);
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
	empty = sem_create("empty", 0);
	ins = lock_create("ins");
	rem = lock_create("rem");
	KASSERT(empty != NULL && ins != NULL && rem != NULL);
	for(int i = 0; i < NCOLOURS; i++){
		KASSERT((mixlocks[i] = lock_create("mixlock")) != NULL);
	}
	hi = lo = 0;
}

/*
 * paintshop_close()
 *
 * Perform any cleanup after the paint shop has closed and everybody
 * has gone home.
 */

void paintshop_close(void)
{
	sem_destroy(empty);
	lock_destroy(ins);
	lock_destroy(rem);
	for(int i = 0; i < NCOLOURS; i++){
		lock_destroy(mixlocks[i]);
	}
}

