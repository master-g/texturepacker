/*
 *  btree.c
 *  texturepacker
 *
 *  Created by Master.G on 13-9-12.
 *  Copyright (c) 2013 Master.G. All rights reserved.
 */

#include <string.h>

#include "texturepacker.h"
#include "memtracker.h"

/*
 * ************************************************************
 * binary tree methods
 * ************************************************************
 */
btree_t *BTree_Create(void)
{
    btree_t *tree = NULL;
    tree = (btree_t *)calloc(1, sizeof(btree_t));
    
    if (tree == NULL)
        printf(MEMORY_ERROR_MSG);
    
    memset(tree, 0, sizeof(btree_t));
    
    return tree;
}

void BTree_DumpNodesToList(btree_t *node, void *context)
{
    list_t **list = (list_t **)context;
    if ((*list) == NULL)
    {
        (*list) = List_Create();
        (*list)->payload.type = Payload_BTreeNode;
        (*list)->payload.data = node;
    }
    else
    {
        list_t *newNode = List_Create();
        newNode->payload.type = Payload_BTreeNode;
        newNode->payload.data = node;
        List_PushBack((*list), newNode);
    }
}

void BTree_Destroy(btree_t *tree)
{
    list_t *list = NULL;
    BTree_Traverse(tree, BTreeTraverse_PostOrder, BTree_DumpNodesToList, &list);
    List_Destroy(list, ListDestroy_PayLoad|ListDestroy_List);
}

void BTree_Traverse(btree_t *tree, int mode, btree_visit_func func, void *context)
{
    if(tree != NULL && func != NULL)
    {
        switch (mode)
        {
            case BTreeTraverse_InOrder:
                BTree_Traverse(tree->left, mode, func, context);
                func(tree, context);
                BTree_Traverse(tree->right, mode, func, context);
                break;
                
            case BTreeTraverse_PreOrder:
                func(tree, context);
                BTree_Traverse(tree->left, mode, func, context);
                BTree_Traverse(tree->right, mode, func, context);
                break;
                
            case BTreeTraverse_PostOrder:
                BTree_Traverse(tree->left, mode, func, context);
                BTree_Traverse(tree->right, mode, func, context);
                func(tree, context);
                break;
                
            default:
                break;
        }
    }
    
    return;
}

list_t *BTree_DumpToList(btree_t *tree, int mode)
{
    list_t *list = NULL;
    BTree_Traverse(tree, BTreeTraverse_PostOrder, BTree_DumpNodesToList, &list);
    return list;
}

void BTree_SetImage(btree_t *node, image_t *image)
{
    image_t *dstImg = node->rect.image;
    if (dstImg != NULL)
    {
        if (dstImg->filename != NULL)
        {
            free(dstImg->filename);
            dstImg->filename = NULL;
        }
        
        free(dstImg);
    }
    
    dstImg = (image_t *)calloc(1, sizeof(image_t));
    Util_CopyString(&dstImg->filename, image->filename);
    
    dstImg->w = image->w;
    dstImg->h = image->h;
    
    node->rect.image = dstImg;
}

btree_t *BTree_Insert(btree_t *root, image_t *image, int padding)
{
    int dw, dh;
    int padwidth, padheight;
    
    if ((root->left != NULL) || (root->right) != NULL)
    {
        if (root->left != NULL)
        {
            btree_t *node = BTree_Insert(root->left, image, padding);
            if (node != NULL)
                return node;
        }
        if (root->right)
        {
            btree_t *node = BTree_Insert(root->right, image, padding);
            if (node != NULL)
                return node;
        }
        
        return NULL;
    }
    
    padwidth = image->w + padding * 2;
    padheight = image->h + padding * 2;
    
    
    if ((padwidth > root->rect.w) || (padheight > root->rect.h))
        return NULL;
    
    dw = root->rect.w - padwidth;
    dh = root->rect.h - padheight;
    
    if (dw <= dh)
    {
        root->left = BTree_Create();
        root->left->rect.x = root->rect.x + padwidth;
        root->left->rect.y = root->rect.y;
        root->left->rect.w = dw;
        root->left->rect.h = padheight;
        
        root->right = BTree_Create();
        root->right->rect.x = root->rect.x;
        root->right->rect.y = root->rect.y + padheight;
        root->right->rect.w = root->rect.w;
        root->right->rect.h = dh;
    }
    else
    {
        root->left = BTree_Create();
        root->left->rect.x = root->rect.x;
        root->left->rect.y = root->rect.y + padheight;
        root->left->rect.w = padwidth;
        root->left->rect.h = dh;
        
        root->right = BTree_Create();
        root->right->rect.x = root->rect.x + padwidth;
        root->right->rect.y = root->rect.y;
        root->right->rect.w = dw;
        root->right->rect.h = root->rect.h;
    }
    
    root->rect.w = padwidth;
    root->rect.h = padheight;
    root->rect.padding = padding;
    BTree_SetImage(root, image);
    
    return root;
}
