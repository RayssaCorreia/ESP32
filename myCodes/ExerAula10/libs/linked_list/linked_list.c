#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include "linked_list.h"

//https://www.tutorialspoint.com/data_structures_algorithms/linked_list_program_in_c.htm

struct node *head = NULL;
struct node *current = NULL;

//display the list
void printList() {
   struct node *ptr = head;
   printf("\n[ ");
	
   //start from the beginning
   while(ptr != NULL) {
      printf("(%d) ",ptr->socket);
      ptr = ptr->next;
   }
	
   printf(" ]\n\r");
}

//insert link at the first location
void insertFirst(int socket) {
   //create a link
   struct node *link = (struct node*) malloc(sizeof(struct node));
	
   link->socket = socket;
	
   //point it to old first node
   link->next = head;
	
   //point first to new first node
   head = link;
}

int length() {
   int length = 0;
   struct node *current;
	
   for(current = head; current != NULL; current = current->next) {
      length++;
   }
	
   return length;
}

//delete a link with given socket
struct node* deleteNode(int socket) {

   //start from the first link
   struct node* current = head;
   struct node* previous = NULL;
	
   //if list is empty
   if(head == NULL) {
      return NULL;
   }

   //navigate through list
   while(current->socket != socket) {

      //if it is last node
      if(current->next == NULL) {
         return NULL;
      } else {
         //store reference to current link
         previous = current;
         //move to next link
         current = current->next;
      }
   }

   //found a match, update the link
   if(current == head) {
      //change first to point to next link
      head = head->next;
   } else {
      //bypass the current link
      previous->next = current->next;
   }    
	
   return current;
}

struct node* getFirst()
{
    return head;
}