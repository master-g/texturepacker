/*
 *  list.c
 *  texturepacker
 *
 *  Created by Master.G on 13-9-12.
 *  Copyright (c) 2013 Master.G. All rights reserved.
 */

#include <string.h>

#include "texturepacker.h"
#include "memtracker.h"

/* create a empty list node */
list_t *List_Create(void)
{
    list_t *list = NULL;
    list = (list_t *)malloc(sizeof(list_t));
    if (list == NULL)
        printf(MEMORY_ERROR_MSG);
    
    memset(list, 0, sizeof(list_t));
    
    return list;
}

void Payload_Destroy(int type, void* data)
{
    rect_t *rect;
    image_t *image;
    btree_t *tree;
    
    switch (type)
    {
        case Payload_Image:
            image = (image_t *)data;
            free(image->filename);
            free(image);
            break;
            
        case Payload_Rect:
            rect = (rect_t *)data;
            free(rect->image->filename);
            free(rect->image);
            free(rect);
            break;
            
        case Payload_BTreeNode:
            tree = (btree_t *)data;
            free(tree);
            if (tree->rect.image != NULL)
            {
                if (tree->rect.image->filename != NULL)
                    free(tree->rect.image->filename);
                
                free(tree->rect.image);
            }
            break;
            
        default:
            break;
    }
}

/* destroy a list with parameter */
void List_Destroy(list_t *list, int mode)
{
    list_t *tmpNode = NULL;
    list_t *node = NULL;
    
    node = list;
    tmpNode = list;
    
    do
    {
        tmpNode = node;
        node = node->next;
        if ((mode & ListDestroy_PayLoad) != 0)
            Payload_Destroy(tmpNode->payload.type, tmpNode->payload.data);
        
        free(tmpNode);
        
    } while (node != NULL && (mode & ListDestroy_List) != 0);
}

/* push a list/node to the back of list */
void List_PushBack(list_t *list, list_t *node)
{
    while (list->next != NULL)
        list = list->next;
    
    list->next = node;
}

/* push a list/node to the beginning of list */
void List_PushFront(list_t **head, list_t *node)
{
    /* find tail */
    list_t *tail = node;
    while (tail->next != NULL)
        tail = tail->next;
    
    /* link */
    tail->next = (*head);
    /* new head */
    (*head) = node;
}

/* get tail of list */
list_t *List_GetTail(list_t *list)
{
    while (list != NULL && list->next != NULL)
        list = list->next;
    
    return list;
}

/*
 * ************************************************************
 * list quick sort
 * ************************************************************
 */

int List_CalculateSortValue(list_t *node)
{
    int value = 0;
    image_t *img;
    rect_t *rect;
    btree_t *btnode;
    
    if (node->payload.data == NULL)
        return 0;
    
    switch (node->payload.type)
    {
        case Payload_Image:
            img = (image_t *)node->payload.data;
            value = img->w * img->h;
            break;
            
        case Payload_Rect:
            rect = (rect_t *)node->payload.data;
            value = rect->w * rect->h;
            break;
            
        case Payload_BTreeNode:
            btnode = (btree_t *)node->payload.data;
            value = btnode->rect.h * btnode->rect.w;
            break;
            
        default:
            break;
    }
    
    return value;
}

/* partition the list using last node as pivot */
list_t *List_Partition(list_t *head, list_t *end, list_t **newHead, list_t **newEnd)
{
    list_t *pivot = end;
    list_t *prev = NULL, *cur = head, *tail = pivot;
    
    /*
     * during partition, both the head and end of the list might change
     * which is updated in the newHead and newEnd
     */
    while (cur != pivot)
    {
        int curArea, pivotArea;
        curArea = List_CalculateSortValue(cur);
        pivotArea = List_CalculateSortValue(pivot);
        
        if (curArea > pivotArea)
        {
            /*
             * first node that has a value greater than the pivot
             * becomes the new head
             */
            if ((*newHead) == NULL)
                (*newHead) = cur;
            
            prev = cur;
            cur = cur->next;
        }
        else /* if cur node is less than pivot */
        {
            list_t *tmp;
            
            /* move cur node to tail */
            if (prev)
                prev->next = cur->next;
            
            tmp = cur->next;
            cur->next = NULL;
            tail->next = cur;
            tail = cur;
            cur = tmp;
        }
    }
    
    /*
     * if the pivot data is the largest element in the list
     * pivot becomes the head
     */
    if ((*newHead) == NULL)
        (*newHead) = pivot;
    
    /* update newEnd to the current tail */
    (*newEnd) = tail;
    
    /* return pivot */
    return pivot;
}

/* sorting without tail node */
list_t *List_QuickSortRecursive(list_t *head, list_t *end)
{
    list_t *newHead = NULL, *newEnd = NULL, *pivot = NULL;
    
    /* base condition */
    if (!head || head == end)
        return head;
    
    /*
     * partition the list, newHead and newEnd will be update
     * by the partition function
     */
    pivot = List_Partition(head, end, &newHead, &newEnd);
    
    /*
     * if pivot is the largest element - no need to recur
     * for the left part
     */
    if (newHead != pivot)
    {
        /* set the node before the pivot node as NULL */
        list_t *tmp = newHead;
        while (tmp->next != pivot)
            tmp = tmp->next;
        tmp->next = NULL;
        
        /* recur for the list before pivot */
        newHead = List_QuickSortRecursive(newHead, tmp);
        
        /* change next of last node of the left half to pivot */
        tmp = List_GetTail(newHead);
        tmp->next = pivot;
    }
    
    /* recur for the list after the pivot element */
    pivot->next = List_QuickSortRecursive(pivot->next, newEnd);
    
    return newHead;
}

/* check if a list is sortable */
int List_IsSortable(list_t *list)
{
    list_t *node = list;
    while (node != NULL)
    {
        if (node->payload.type != Payload_Image)
        {
            printf("Invalid payload for a sortable list\n");
            return 0;
        }
        
        node = node->next;
    }
    
    return 1;
}

void List_Sort(list_t **list)
{
    (*list) = List_QuickSortRecursive(*list, List_GetTail(*list));
    return;
}

/* ************************************************************ */

void List_Print(list_t *list)
{
    list_t *node = list;
    image_t *img = NULL;
    btree_t *tree = NULL;
    
    while (node != NULL)
    {
        printf("List : %p\n", (void *)node);
        switch (node->payload.type)
        {
            case Payload_Image:
                img = (image_t *)(node->payload.data);
                printf("    ");
                printf("Image w:%5d h:%5d file:%s\n", img->w, img->h, img->filename);
                break;
                
            case Payload_Rect:
                printf("    ");
                Rect_Print((rect_t *)node->payload.data);
                break;
                
            case Payload_BTreeNode:
                printf("    ");
                tree = (btree_t *)node->payload.data;
                Rect_Print(&tree->rect);
                break;
                
            default:
                break;
        }
        node = node->next;
    }
}
