#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <assert.h>
#include "network.h"
#include "interrupts_private.h"
#include "minithread.h"
#include "machineprimitives.h"
#include "random.h"

#define MAXEVENTS 64
#define MAXBUF 1000
#define ENABLED 1
#define DISABLED 0
#define FPSTATE_SIZE (512)

#define DISK_INTERRUPT_TYPE 4
#define READ_INTERRUPT_TYPE 3
#define NETWORK_INTERRUPT_TYPE 2
#define CLOCK_INTERRUPT_TYPE 1

long ticks;
extern unsigned int start();
extern unsigned int end();
interrupt_level_t interrupt_level;

static pthread_mutex_t signal_mutex;

typedef struct interrupt_t interrupt_t;
struct interrupt_t {
  interrupt_handler_t handler;
  void *arg;
};

interrupt_handler_t mini_clock_handler;
interrupt_handler_t mini_network_handler;
interrupt_handler_t mini_read_handler;
interrupt_handler_t mini_disk_handler;

static volatile int signal_handled = 0;
static volatile interrupt_t interrupt;

static sem_t *interrupt_received_sema; 

#define RAX 0
#define RBX 1
#define RCX 2
#define RDX 3
#define RDI 4
#define RSI 5
#define RBP 6
#define RSP 7
#define R8 8
#define R9 9
#define R10 10
#define R11 11
#define R12 12
#define R13 13
#define R14 14
#define R15 15
#define RIP 16

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); \
} while (0)

/*
 * atomically sets interrupt level and returns the original
 * interrupt level
 */
interrupt_level_t set_interrupt_level(interrupt_level_t newlevel) {
  return swap(&interrupt_level, newlevel);
}

void gen_random_string(char *s, const int len) {
  int i;
  static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

  for (i = 0; i < len - 1; ++i) {
    s[i] = alphanum[genintrand(sizeof(alphanum) - 1)];
  }

  s[len - 1] = '\0';
}

/*
 * Register the minithread clock handler by making
 * mini_clock_handler point to it.
 *
 * Then set the signal handler for SIGRTMIN+1 to 
 * handle_interrupt.  This signal handler will either
 * interrupt the minithreads, or drop the interrupt,
 * depending on safety conditions.
 *
 * The signals are handled on their own stack to reduce
 * chances of an overrun.
 */
void 
minithread_clock_init(interrupt_handler_t clock_handler){
  struct itimerval its;
  struct sigaction sa;
  stack_t ss;
  char sem_name[16];
  mini_clock_handler = clock_handler;

  do{
    gen_random_string(sem_name,16);
    interrupt_received_sema = sem_open(sem_name ,O_CREAT | O_EXCL, O_RDWR, 0);
  } while((long)interrupt_received_sema == -1);

  ss.ss_sp = malloc(SIGSTKSZ);
  if (ss.ss_sp == NULL){
    perror("malloc."); 
    abort();
  }
  ss.ss_size = SIGSTKSZ;
  ss.ss_flags = 0;
  if (sigaltstack(&ss, NULL) == -1){
    perror("signal stack");
    abort();
  }


  /* Establish handler for timer signal */
  sa.sa_handler = (void*)handle_interrupt;
  sa.sa_flags = SA_SIGINFO | SA_RESTART | SA_ONSTACK; 
  sa.sa_sigaction= (void*)handle_interrupt;
  sigemptyset(&sa.sa_mask);
  sigaddset(&sa.sa_mask,CLOCK_SIGNAL);
  sigaddset(&sa.sa_mask,NETWORK_SIGNAL);
  if (sigaction(CLOCK_SIGNAL, &sa, NULL) == -1)
    errExit("sigaction");

  /* Start the timer */
  its.it_value.tv_sec = 0;
  its.it_value.tv_usec = PERIOD;
  its.it_interval.tv_sec = its.it_value.tv_sec;
  its.it_interval.tv_usec = its.it_value.tv_usec;
  if(setitimer(ITIMER_VIRTUAL,&its,NULL)==-1)
    errExit("setitimer");
}


/*
 * This function handles a signal and invokes the specified interrupt
 * handler, ensuring that signals are unmasked first.
 */
  void
handle_interrupt(int sig, siginfo_t *si, ucontext_t *ucontext)
{

  unsigned long *gr, *fp;
  uint64_t eip;

  gr = (unsigned long *)&ucontext->uc_mcontext->__ss;
  fp = (unsigned long *)&ucontext->uc_mcontext->__fs;
  ++fp; /*There are 8 bytes of empty space we don't want*/
  eip = gr[RIP];

  /*
   * This allows us to check the interrupt level
   * and effectively block other signals.
   *
   * We can check the interrupt level and disable
   * them, which will prevent other signals from
   * interfering with our other check for library
   * calls.
   */
//  printf("start = %p\n",start);
//  printf("end = %p\n",end);
  if(interrupt_level==ENABLED &&
      eip > (unsigned long)start &&
      eip < (unsigned long)end){

    unsigned long *newsp;
    /*
     * push the return address
     */
    newsp = (unsigned long *) gr[RSP];
    *--newsp =(unsigned long) gr[RIP];

    /*
     * make room for saved state and align stack.
     */
#define ROUND(X,Y)   (((unsigned long)X) & ~(Y-1)) /* Y must be a power of 2 */
    newsp = (unsigned long *) ROUND(newsp, 16);
    if(fp!=0){
      newsp -= 512/sizeof(unsigned long);
      memcpy(newsp,fp,512);
      fp = newsp;
    } else {
      AbortOnCondition(1,"No FP state...");
    }

    --newsp; 
    *--newsp = (unsigned long) gr[RSP] - sizeof(unsigned long); /*address of RIP*/
    newsp -= sizeof(struct __darwin_x86_thread_state64)/sizeof(unsigned long);
    memcpy(newsp,gr,sizeof(struct __darwin_x86_thread_state64));
    *--newsp = (unsigned long)fp; 
    *--newsp = (unsigned long)minithread_trampoline; /*return address*/

    /*
     * set the context so that we end up in the student's clock handler
     * and our stack pointer is at the return address we just pushed onto
     * the stack.
     */
    if(sig!=CLOCK_SIGNAL){
        if(DEBUG)
            printf("Signal received\n");
        gr[RSP]=(unsigned long)newsp; 
        gr[RIP]=(unsigned long)interrupt.handler;
        gr[RDI]=(unsigned long)interrupt.arg;

        interrupt.arg = NULL;
        signal_handled = 1;
        set_interrupt_level(DISABLED);
    }
    else{
      gr[RSP]=(unsigned long)newsp; 
      gr[RIP]=(unsigned long)mini_clock_handler;
      gr[RDI]=(unsigned long)0;
      if(DEBUG)
        printf("SP=%p\n",newsp);
    }
  }
  if(sig!=CLOCK_SIGNAL)
    sem_post(interrupt_received_sema);
}

void send_interrupt(int interrupt_type, interrupt_handler_t handler, void* arg){

    pthread_mutex_lock(&signal_mutex);
    for (;;){
      
        signal_handled = 0;

        /* Repeat if signal is not delivered. */
        /* OSX allows sigaction style signal handlers, but provides
         * no way of actually sending a value with the signal, so 
         * we need to just pass it in a global.  A hack, but it is
         * correct .
         */
        interrupt.arg = arg;
        if(interrupt_type==NETWORK_INTERRUPT_TYPE)
            interrupt.handler = mini_network_handler;
        else if(interrupt_type==READ_INTERRUPT_TYPE)
            interrupt.handler = mini_read_handler; 
        else if(interrupt_type==DISK_INTERRUPT_TYPE)
            interrupt.handler = mini_disk_handler;
        else
            abort();

        AbortOnCondition(kill(getpid(),NETWORK_SIGNAL)==-1,
            "Could not send signal.");
        
        /* semaphore_P to wait for main thread signal */
        sem_wait(interrupt_received_sema);

        /* Check if interrupt was handled */
        if(signal_handled){
            break;
        }
        
        sleep(0);
        /* resend if necessary */
    }
    pthread_mutex_unlock(&signal_mutex);
}
