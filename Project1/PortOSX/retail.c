
/*
Finally, you need to implement a simple "retail store" application that uses your minithread, 
queue, and semaphore implementations. The application models customers and employees at an 
electronics retail store as threads. Your application should make the best possible use of 
the components you developed in this project to simulate the following:

1) It is release day for the new PortPhone 5, the latest device running the revolutionary 
PortOS from Sirer Systems. Customers are flocking to their local retail store to purchase 
the new phones. Unfortunately, the new shipment of phones has just arrived and employees 
are struggling to keep up with demand.
2) There are N employees . Each employee constantly unpacks phones. Each phone has a unique 
serial number, and the phones are unpacked in increasing serial order.
3) There are M customers, who are served on a first come, first served basis. When a 
customer arrives at the store, they immediately receive a new phone if one is available. 
Otherwise, they wait in line until more phones become available. Ensure, for sanity
4) When a customer receives a phone, (s)he immediately activates it by printing the unique 
serial number to stdout.
*/

// Simple Retail Store application

#include "minithread.h"
#include "semaphore.h"
#include <stdio.h>
#include <stdlib.h>

#define N 3
#define M 5

int serial = 0;
semaphore_t sem;
semaphore_t writesem;
queue_t q;

int customer(int cust) 
{
    semaphore_P(sem);
    printf("Customer %d has serial %d\n", cust, serial);
    return 0;
}

int employee(int arg) 
{
    while(1)
    {
        semaphore_V(sem);
        serial ++;
        minithread_yield();
    }
    return 0;
}

int retail(int* arg)
{
    int emp = 1;
    while(emp <= N)
    {
        minithread_t thread = minithread_fork(&employee, emp);
        emp ++;
    }  
    int cust = 1;
    while(cust <= M)
    {
        minithread_t thread = minithread_fork(&customer, cust);
        cust ++;
    }
	return 0;
}

int main(void)
{
	sem = semaphore_create();
	semaphore_initialize(sem, N);
  semaphore_initialize(writesem, 1);
  q = queue_new();
	minithread_system_initialize(retail, NULL);
	return 0;
}


