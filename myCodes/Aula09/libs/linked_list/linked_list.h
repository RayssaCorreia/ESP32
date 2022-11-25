#ifndef LINKED_LIST_H_
#define LINKED_LIST_H_
//https://www.tutorialspoint.com/data_structures_algorithms/linked_list_program_in_c.htm

struct node {
   int socket;
   struct node *next;
};

void printList();
void insertFirst(int socket);
int length() ;
struct node* deleteNode(int socket);
struct node* getFirst();
#endif