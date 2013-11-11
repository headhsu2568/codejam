#include <stdio.h>
#include <stdlib.h>

struct Node {
    struct Node* next;
    struct Node* prev;
    int data;
};

struct Node* to_root(struct Node* linkedlist);
void iterate(struct Node* linkedlist);
struct Node* new_linked_list();
void delete_linked_list(struct Node* linkedlist);
struct Node* insert(struct Node* linkedlist, int data);
struct Node* delete(struct Node* linkedlist);
void reverse(struct Node* linkedlist);

struct Node* to_root(struct Node* linkedlist) {
    while(linkedlist->prev) linkedlist = linkedlist->prev;
    return linkedlist;
}

void iterate(struct Node* linkedlist) {
    linkedlist = to_root(linkedlist);
    while(linkedlist) {
        printf("[%d]", linkedlist->data);
        if(linkedlist->next) printf(" <-> ");
        else printf("\n");
        linkedlist = linkedlist->next;
    }
}

struct Node* new_linked_list() {
    struct Node* root = malloc(sizeof(struct Node));
    root->next = NULL;
    root->data = 0;
    root->prev = NULL;
    return root;
}

void delete_linked_list(struct Node* linkedlist) {
    if(linkedlist) {
        linkedlist = to_root(linkedlist);
        struct Node* ptr = NULL;
        while(linkedlist) {
            ptr = linkedlist->next;
            //printf("delete node: %d\n", linkedlist->data);
            free(linkedlist);
            linkedlist = ptr;
        }
    }
    else printf("the linked list is already deleted\n");
}

struct Node* insert(struct Node* linkedlist, int data) {
    struct Node* ptr = malloc(sizeof(struct Node));
    ptr->data = data;
    ptr->prev = linkedlist;
    ptr->next = linkedlist->next;

    linkedlist->next = ptr;
    if(ptr->next) ptr->next->prev = ptr;

    /*
    int prev = -1;
    int next = -1;
    if(ptr->prev) prev = ptr->prev->data;
    if(ptr->next) next = ptr->next->data;
    printf("insert: %d (prev: %d, next: %d)\n", ptr->data, prev, next);
    */

    return ptr;
}

struct Node* delete(struct Node* linkedlist) {
    if(linkedlist->prev) linkedlist->prev->next = linkedlist->next;
    if(linkedlist->next) linkedlist->next->prev = linkedlist->prev;

    struct Node* ptr = NULL;
    if(linkedlist->prev) ptr = linkedlist->prev;
    else if(linkedlist->next) ptr = linkedlist->next;

    //printf("delete: %d\n", linkedlist->data);
    free(linkedlist);

    return ptr;
}

void reverse(struct Node* linkedlist) {
    linkedlist = to_root(linkedlist);
    while(linkedlist) {
        struct Node* prev = linkedlist->prev;
        linkedlist->prev = linkedlist->next;
        linkedlist->next = prev;
        linkedlist = linkedlist->prev;
    }
}

int main() {
    struct Node* linkedlist = new_linked_list();
    int i = 1;
    for(; i < 10; ++i) {
        linkedlist = insert(linkedlist, i);
    }
    iterate(linkedlist);

    linkedlist = to_root(linkedlist);
    while(1) {
        if(linkedlist->data > 4 && linkedlist->data < 8) linkedlist = delete(linkedlist);

        if(linkedlist->next) linkedlist = linkedlist->next;
        else break;
    }
    iterate(linkedlist);

    reverse(linkedlist);
    iterate(linkedlist);

    delete_linked_list(linkedlist);
    return 0;
}
