#include<stdlib.h>
#include "my_allocator.h"

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

// Don't forget to implement "init_allocator()" and "release_allocator()"!
Addr space;
int bbs;
int ml;

// Linked list node, doubly ?
// int size, pointer to allocated space, pointer to buddy, allocation bolean, next node
struct node{
  int size;
  struct node *next;
  // struct node *buddy;
  bool allocated;
  Addr al_sp;
};
struct node *head;
struct node *current;

//display the list
void printList() {

  struct node *ptr = head;
  printf("[");
	
  while(ptr != NULL) {
    printf("(%d,%d,%d)",ptr->size,ptr->allocated,ptr->al_sp);
    ptr = ptr->next;
  }
  printf("] \n");
}

struct node* findbuddy(struct node *ptr)
{
  int x = ptr->al_sp;
  // printf("x = %d \n",x);
  int i;
  int n;

  int blocksize = ptr->size;
  // blocksize *= 2;
  // int start = 2;
  // int counter = 0;
  // while (start != blocksize)
  // {
  //   start *= 2;
  //   counter++;
  // }
  // n = counter+1;

  // i = n-1;
  // x = x ^ (1<<i);

  x -= blocksize;
  x = (Addr) x;

  // printf("x = %d \n",x);

  struct node *temp = head;
	struct node *result = NULL;
  while(temp != NULL) {
    // printf("address: %d \n",(int)temp->al_sp);
    if ((int) temp->al_sp == (int) x)
    {
      // printf("Found a match!!!! \n");
      result = temp;
    }
    temp = temp->next;
  }
  if (result != NULL && result->size == ptr->size)
  {
    // printf("Buddy found \n");
    return result;
  }
  else
  {
    // printf("Buddy not found \n");
    x -= blocksize;
    temp = head;
    // result = NULL;
    // while(temp != NULL) {
    //   // printf("address: %d \n",(int)temp->al_sp);
    //   if ((int) temp->al_sp == (int) x)
    //   {
    //     // printf("Found a match!!!! \n");
    //     result = temp;
    //   }
    //   temp = temp->next;
    // }
    // if (result != NULL && result->size == ptr->size)
    // {
    //   // printf("Buddy found \n");
    //   return result;
    // }
    // else
    // {
    //   // printf("Buddy not found \n");
    //   return ptr;
    // }
    return ptr;
  }

}
// Split
void split(unsigned int _length)
{
  int target = 2*_length;
  if (target > ml)
  {
    printf("There is no block large enough \n");
    exit(1);
  }
  // printf("splitting with target size %d: \n",target);
  // printList();
  struct node *ptr = head;
  struct node *target_node = NULL;
  while(ptr != NULL) {
    if (ptr->allocated == false && ptr->size == target){
      target_node = ptr;
    }
    ptr = ptr->next;
  }
  if (target_node == NULL)
  {
    // should try splitting larger block
    // printf("Matching node not found, searching for larger block \n");
    split(target);
  }
  else
  {
    // printf("splitting a matched block \n");
    int old_size = target_node->size;
    // new node
    struct node *temp = (struct node*) malloc(sizeof(struct node));
    temp->size = old_size/2;
    temp->next = target_node->next;
    // temp->buddy = target_node;
    temp->allocated = false;
    
    // printf("Before halfway calculations \n");
    Addr halfway = target_node->al_sp + old_size/2;
    temp->al_sp = halfway;
    // printf("After halfway calculations \n");

    // edit target_node
    target_node->size = old_size/2;
    target_node->next = temp;
    // target_node->buddy = temp;

    // printf("done with splitting a matched block \n");
  }
}

// coalesce
void coalesce(struct node* ptr)
{
  // printf("coalesce called with size = %d\n",ptr->size);
  // printList();
  struct node *buddy = findbuddy(ptr);
  if (buddy != ptr)
  {
    int old_size = ptr->size;
    // printf("returned from buddy finder \n");
    int buddy_size = buddy->size;
    // printf("buddy's size = %d \n",buddy_size);
    
    if (buddy->allocated == false && old_size == buddy_size)
    {
      // printf("Found empty buddy \n");
      if (ptr->next == buddy)
      {
        ptr->next = buddy->next;
        ptr->size = old_size*2;
        // find new buddy of combined block
        // ptr->buddy = findbuddy(ptr);
        // printf("Buddy size = %d \n",ptr->buddy->size);
        // printf("Buddy is after \n");
        coalesce(ptr);
      } 
      else if (buddy->next = ptr)
      {
        buddy->next = ptr->next;
        buddy->size = old_size*2;
        // find new buddy of combined block
        // buddy->buddy = findbuddy(buddy);
        // printf("Buddy size = %d \n",ptr->buddy->size);
        // printf("Buddy is before \n");
        coalesce(buddy);
      }
      
    }
    else
    {
      return;
    }
    
  }
  else
  {
    // printf("Done with coalescing \n");
    return;
  }
} 


unsigned int init_allocator(unsigned int _basic_block_size, unsigned int _length)
{
  if (_length < _basic_block_size)
  {
    return 0;
  }
  bbs = _basic_block_size;
  
  int real_length = _basic_block_size;
  
  while(real_length < _length)
  {
    real_length *= 2;
  }
  if (real_length != _length)
  {
    real_length = real_length/2;
  }
  ml = real_length;
  space = malloc((size_t)real_length);


  struct node *temp = (struct node*) malloc(sizeof(struct node));
  temp->size = real_length;
  temp->next = NULL;
  // temp->buddy = NULL;
  temp->allocated = false;
  temp->al_sp = space;
  head = temp;

  return real_length;
}

int release_allocator()
{
  printf("Exiting... \n");
  printList();
  free(space);
  return 0;
}

Addr allocate(unsigned int _length)
{

  // traverse through list and find node with size _length and return pointer to allocated space of that node
  struct node *ptr = head;
	struct node *correct = NULL;
  while(ptr != NULL) {
    if (ptr->allocated == false && ptr->size == _length){
      correct = ptr;
    }
    ptr = ptr->next;
  }
  
  if (correct == NULL){
    // printf("should split \n");
    split(_length);
    // printf("done with splitting \n");
    // printList();
    correct = allocate(_length);
  }
  else
  {
    return correct;
  }
  
}

Addr my_malloc(unsigned int _length) {
/* This preliminary implementation simply hands the call over the 
    the C standard library! 
    Of course this needs to be replaced by your implementation.
*/
  // printf("malloc called with size %d \n", _length);
  // printList();
  struct node *temp;

  int best_match = bbs;
  while (best_match < _length)
  {
    best_match *= 2;
  }

  temp = allocate(best_match);
  temp->allocated = true;

  
  // printf("After allocation \n");
  // printList();
  return temp->al_sp;
  // return malloc((size_t)best_match);
}

int my_free(Addr _a) {
  /* This preliminary implementation simply hands the call over the 
     the C standard library! 
     Of course this needs to be replaced by your implementation.
  */
  // free(_a);
  // printf("Trying to free... \n");
  // printList();

  // look for node storing this allocted space
  // free up node
  struct node *ptr = head;
  struct node *target_node = NULL;
  while(ptr != NULL) {
    if (ptr->al_sp == _a ){
      int s = ptr->size;
      // printf("Block found with size %d\n",s);
      target_node = ptr;
      target_node->allocated = false;
      // printList();
    }
    ptr = ptr->next;
  }
  if (target_node == NULL)
  {
    printf("No such allocated block found \n");
  }
  else
  {
    coalesce(target_node);
  }

  // printList();
  return 0;
}

