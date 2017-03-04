#include <iostream>
#include "linkedList.h"
#include <cassert>
#include <stdlib.h>

using namespace std;

void addMiddleNode(LinkedList *list, int position);
void addIntToStartOfListLab(LinkedList *list, int value);
void addIntToStartOfListTest(LinkedList *list, int value);

LinkedList * arrayToLinkedList(int *a, int size) {
  
  LinkedList * list = new LinkedList;

  list->head=NULL; 
  list->tail=NULL;

  for (int i=0; i<size; i++) {
    // add array[i] to the list

    if ( list->head==NULL) {
      list->head = new Node;
      list->head->data = a[i]; // (*head).data = a[i];
      list->head->next = NULL;
      list->tail = list->head;
    } else {
      list->tail->next = new Node;
      list->tail = list->tail->next;
      list->tail->next = NULL;
      list->tail->data = a[i];
    }
  }

  return list; // return ptr to new list

}

void addIntToStartOfListLab(LinkedList *list, int value) {//This is my lab function
  assert(list!=NULL); // if list is NULL, we can do nothing.
  int initialdata, postdata, lastdata; // Remembers original values so we can reuse them
  const Node* j=list->head;
  if (list->head == NULL) {
    list->head = new Node;
    list->head->data = value;
    list->tail = list->head;
    list->tail->next = NULL;     
  } else {
     initialdata = list->head->data;
     lastdata = list->tail->data;
     while(j->next!=NULL) {
	postdata = initialdata;
	initialdata =  j->next->data;
	j->next->data = postdata;
        j = j->next;		
    }
    list->tail->next = new Node;
    list->tail->next->data = lastdata;
    list->tail = list->tail->next;
    list->tail->next = NULL; 
    list->head->data = value;
    } 
}

void addIntToStartOfListTest(LinkedList *list, int value) {//This is my test function
   assert(list!=NULL);//Changed my for loop to assert
   int initial, post;
   list->tail->next = new Node;
   list->tail = list->tail->next;
   list->tail->next = NULL;
   list->tail->data = 1;
   initial = list->head->data;
   for (Node *i = list->head;i->next!=NULL; i = i->next){
	post = initial;
	initial= i->next->data;
	i->next->data = post;
    }
   list->head->data = value;
}

int main(){
  int arr[5]= {1,2,3,4,5};
  int arr1[5] = {1,2,3,4,5};
  int arr2[1] = {1};
  int arr3[0] = {};
  LinkedList * list = arrayToLinkedList(arr,5);
  LinkedList * list1 = arrayToLinkedList(arr1,5);
  LinkedList * list2 = arrayToLinkedList(arr2,1);
  LinkedList * list3 = arrayToLinkedList(arr3,0);
  cout << "original linked list: ";
  for (Node*  j= list->head;j!=NULL;j= j->next){
	 cout << j->data << " ";
  }
  cout << endl;
  addIntToStartOfListLab (list, 4);
  cout << "linked list after lab function: ";
  for (Node*  j= list->head;j!=NULL;j= j->next){
	 cout << j->data << " ";
  }
  cout << endl;
  addIntToStartOfListTest (list1, 4);
  cout << "linked list after test function(5): ";
  for (Node*  j= list1->head;j!=NULL;j= j->next){
	 cout << j->data << " ";
  }
  cout << endl;
  addIntToStartOfListTest (list2, 4);
  cout << "linked list after test function (1): ";
  for (Node*  j= list2->head;j!=NULL;j= j->next){
	 cout << j->data << " ";
  }
  cout << endl;
  addIntToStartOfListTest (list3, 4);
  cout << "linked list after test function (0): ";
  for (Node*  j= list3->head;j!=NULL;j= j->next){
	 cout << j->data << " ";
  }
  cout << endl;
  return 0;
}
