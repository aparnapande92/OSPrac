
#include "queue.h"


#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int func(int* x, int* y)
{
  if (x==NULL) 
  {
    printf("elem:%d\n", *y);
  }
  else 
  {
    printf("%d:elem:%d\n",*x, *y);
  }
  return 0;
}

int main(void) 
{
  //Declaring
  int* deitem; 
  int result;
  queue_t q;
  void* item;
  void* item2;
  int* resitem;
  int* resitem2;
  int* resitem3;
  int i;


  // Initializing
  q  = queue_new();
  item = 19;
  item2 = item + 3;

  //resitem = (int*) item;
  
  
  result = queue_prepend(q, &item);
  result = queue_prepend(q, &item2);

  i=1;
  result = queue_iterate(q, func, &i);
  printf("length: %d\n", queue_length(q));
  
  
  queue_dequeue(q, &deitem);
  i=3;
  result = queue_iterate(q, func, &i);
  queue_prepend(q, deitem);
  i=2;
  result = queue_iterate(q, func, &i);

  result = queue_delete(q, &deitem);
  //while (queue_dequeue(q, &deitem) != -1) {}
  printf("nnnnn%d\n", result);
  
  i=2;
  result = queue_iterate(q, func, &i);
  
  printf("length: %d\n", queue_length(q));
  
  //result = queue_free(q);
  //sleep(2);

  printf("length: %d\n", queue_length(q));
  resitem = q->length; 
  printf(",,,, %d", resitem);



  return 0;
}
