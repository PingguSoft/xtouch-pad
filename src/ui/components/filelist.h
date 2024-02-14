#ifndef __FILE_LIST__
#define __FILE_LIST__

#include <stdio.h>
#include <stdlib.h>
#include "../ui.h"

struct FileInfo {
    int    id;
    char   *pngName;
    char   *modelName;
};

struct FileNode {
    struct FileInfo info;
    struct FileNode* next;
    struct FileNode* prev;
};

struct FileNode* _head = NULL;

// insert node at the front
void insertFront(struct FileNode** head, struct FileInfo *info) {
    // allocate memory for newNode
    struct FileNode* newNode = (struct FileNode*)lv_mem_alloc(sizeof(struct FileNode));
    newNode->info = *info;

    // make newNode as a head
    newNode->next = (*head);

    // assign null to prev
    newNode->prev = NULL;

    // previous of head (now head is the second node) is newNode
    if ((*head) != NULL)
        (*head)->prev = newNode;

    // head points to newNode
    (*head) = newNode;
}

// insert a node after a specific node
void insertAfter(struct FileNode* prev_node, struct FileInfo *info) {
    // check if previous node is null
    if (prev_node == NULL) {
        LOGD("previous node cannot be null");
        return;
    }

    // allocate memory for newNode
    struct FileNode* newNode = (struct FileNode*)lv_mem_alloc(sizeof(struct FileNode));
    newNode->info = *info;

    // set next of newNode to next of prev node
    newNode->next = prev_node->next;

    // set next of prev node to newNode
    prev_node->next = newNode;

    // set prev of newNode to the previous node
    newNode->prev = prev_node;

    // set prev of newNode's next to newNode
    if (newNode->next != NULL)
        newNode->next->prev = newNode;
}

// insert a newNode at the end of the list
void insertEnd(struct FileNode** head, struct FileInfo *info) {
    // allocate memory for node
    struct FileNode* newNode = (struct FileNode*)lv_mem_alloc(sizeof(struct FileNode));
    newNode->info = *info;

    // assign null to next of newNode
    newNode->next = NULL;

    // store the head node temporarily (for later use)
    struct FileNode* temp = *head;

    // if the linked list is empty, make the newNode as head node
    if (*head == NULL) {
        newNode->prev = NULL;
        *head = newNode;
        return;
    }

    // if the linked list is not empty, traverse to the end of the linked list
    while (temp->next != NULL)
        temp = temp->next;

    // now, the last node of the linked list is temp

    // assign next of the last node (temp) to newNode
    temp->next = newNode;

    // assign prev of newNode to temp
    newNode->prev = temp;
}

// delete a node from the doubly linked list
void deleteNode(struct FileNode** head, struct FileNode* del_node) {
    // if head or del is null, deletion is not possible
    if (*head == NULL || del_node == NULL)
        return;

    // if del_node is the head node, point the head pointer to the next of del_node
    if (*head == del_node)
        *head = del_node->next;

    // if del_node is not at the last node, point the prev of node next to del_node to the previous of del_node
    if (del_node->next != NULL)
        del_node->next->prev = del_node->prev;

    // if del_node is not the first node, point the next of the previous node to the next node of del_node
    if (del_node->prev != NULL)
        del_node->prev->next = del_node->next;

    // lv_mem_free the memory of del_node
    if (del_node->info.pngName)
        lv_mem_free(del_node->info.pngName);
    lv_mem_free(del_node);
}

void deleteNodeWithID(int id) {
    struct FileNode* temp = _head;

    while (temp != NULL) {
        if (id == temp->info.id) {
            deleteNode(&_head, temp);
            break;
        }
        temp = temp->next;
    }
}

void deleteAllNodes() {
    struct FileNode* temp = _head;

    // if the linked list is not empty, traverse to the end of the linked list
    while (temp != NULL) {
        deleteNode(&_head, temp);
        temp = temp->next;
    }
}

void displayList(struct FileNode* node) {
    while (node != NULL) {
        LOGD("%s -> ", node->info.pngName);
        node = node->next;
    }
    if (node == NULL)
        LOGD("NULL\n");
}

#endif
