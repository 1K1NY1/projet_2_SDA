/* ========================================================================= *
 * List definition
 * ========================================================================= */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include "List.h"

List *listNew(void)
{
    List *l = malloc(sizeof(List));
    if (!l)
        return NULL;
    l->head = NULL;
    l->last = NULL;
    l->size = 0;
    return l;
}

void listFree(List *l, bool freeContent)
{
    // Free LNodes
    LNode *node = l->head;
    LNode *prev = NULL;
    while (node != NULL)
    {
        prev = node;
        node = node->next;
        if (freeContent)
            free(prev->value);
        free(prev);
    }
    // Free LinkedList sentinel
    free(l);
}

size_t listSize(List *l)
{
    return l->size;
}

bool listInsertLast(List *l, void *value)
{
    LNode *node = malloc(sizeof(LNode));
    if (!node) {
        fprintf(stderr, "listInsertLast: allocation error.\n");
        exit(EXIT_FAILURE);
    }
    // Initialisation
    node->next = NULL;
    node->value = value;
    // Adding the node to the list
    if (!l->last)
    {
        // First element in the list
        l->last = node;
        l->head = node;
    }
    else
    {
        // At least one element in the list
        l->last->next = node;
        l->last = node;
    }
    // In both cases, increment size
    l->size++;
    return true;
}

bool listInsertFirst(List *l, void *value)
{
    LNode *node = malloc(sizeof(LNode));
    if (!node) {
        fprintf(stderr, "listInsertFirst: allocation error.\n");
        exit(EXIT_FAILURE);
    }

    // Initialisation
    node->next = NULL;
    node->value = value;
    // Adding the node to the list
    if (!l->head)
    {
        // First element in the list
        l->head = node;
        l->last = node;
    }
    else
    {
        // At least one element in the list
        node->next = l->head;
        l->head = node;
    }
    // In both cases, increment size
    l->size++;
    return true;
}
