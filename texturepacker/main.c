/*
 *  main.c
 *  texturepacker
 *
 *  Created by Master.G on 13-9-12.
 *  Copyright (c) 2013 Master.G. All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "memtracker.h"
#include "texturepacker.h"

void test_algorithm(void)
{
    int padding = 1;
    list_t *imageList = NULL;
    btree_t *tree;
    list_t *treeDumpList = NULL;
    list_t *fileListIter = NULL;
    int i = 0;
    char filename[10];
    int success = 1;
    int initsize = 512;
    
    srand((unsigned int)time(NULL));
        
    for (i = 0; i < 2800; i++)
    {
        list_t *node = NULL;
        image_t *img = NULL;
        img = (image_t *)calloc(1, sizeof(image_t));
        node = List_Create();
        
        sprintf(filename, "%d.png", i);
        
        img->w = 5 + rand() % 20;
        img->h = 5 + rand() % 20;
        Util_CopyString(&img->filename, filename);
        
        node->payload.type = Payload_Image;
        node->payload.data = img;
        
        List_PushFront(&imageList, node);
    }
    
    List_Sort(&imageList);
    
    tree = BTree_Create();
    tree->rect.x = 0;
    tree->rect.y = 0;
    tree->rect.w = initsize;
    tree->rect.h = initsize;
    
    do
    {
        success = 1;
        
        for (fileListIter = imageList; fileListIter != NULL; fileListIter = fileListIter->next)
        {   
            if (BTree_Insert(tree, (image_t *)fileListIter->payload.data, padding) == NULL)
            {
                printf("Shit happens\n");
                success = 0;
                break;
            }
        }
        
        if (!success)
        {
            BTree_Destroy(tree);
            tree = BTree_Create();
            tree->rect.x = 0;
            tree->rect.y = 0;
            initsize = Util_NextPOT(initsize+1);
            tree->rect.w = initsize;
            tree->rect.h = initsize;
        }
        
    } while (!success);
    
    printf("-----\n");
    List_Destroy(imageList, ListDestroy_List|ListDestroy_PayLoad);
    
    treeDumpList = BTree_DumpToList(tree, BTreeTraverse_PostOrder);
    
    {
        list_t *iter = NULL;
        bitmap_t *canvas = NULL;
        rect_t *rect = NULL;
        
        canvas = Bitmap_Create(initsize, initsize);
        
        iter = treeDumpList;
        for (iter = treeDumpList; iter != NULL; iter = iter->next)
        {
            rect = (rect_t *)iter->payload.data;
            if (rect->image != NULL && rect->image->filename != NULL)
            {
                int argb = 0;
                bitmap_t *pad = NULL;
                bitmap_t *png = NULL;
                png = Bitmap_Create(rect->image->w, rect->image->h);
                pad = Bitmap_Create(rect->image->w + rect->padding * 2, rect->image->h + rect->padding * 2);
                argb = rand();
                argb |= 0xFF000000;
                Bitmap_Clean(png, argb);
                Bitmap_Clean(pad, 0);
                Bitmap_CopyPasteBitmap(pad, png, rect->padding, rect->padding);
                Bitmap_CopyPasteBitmap(canvas, pad, rect->x, rect->y);
                Bitmap_Destroy(png);
                Bitmap_Destroy(pad);
            }
        }
        
        Bitmap_WriteAsPNG(canvas, "canvas.png");
        Bitmap_Destroy(canvas);
    }
    
    List_Destroy(treeDumpList, ListDestroy_List);
    
    if (tree != NULL)
        BTree_Destroy(tree);
    
    memtrack_list_allocations();
}

void packpng(void)
{
    list_t *pngFileList = NULL;
    list_t *fileListIter = NULL;
    btree_t *tree = NULL;
    list_t *treeDumpList = NULL;
    
    pngFileList = Util_ReadAllPNGs();
    
    tree = BTree_Create();
    tree->rect.x = 0;
    tree->rect.y = 0;
    tree->rect.w = 512;
    tree->rect.h = 512;
    
    for (fileListIter = pngFileList; fileListIter != NULL; fileListIter = fileListIter->next)
    {
        if (BTree_Insert(tree, (image_t *)fileListIter->payload.data, 0) == NULL)
        {
            printf("Enlarge needed\n");
        }
    }
    
    treeDumpList = BTree_DumpToList(tree, BTreeTraverse_PreOrder);
    printf("-----\n");
    List_Print(treeDumpList);
    
    {
        list_t *iter = NULL;
        bitmap_t *canvas = NULL;
        rect_t *rect = NULL;
        
        canvas = Bitmap_Create(512, 512);
        
        iter = treeDumpList;
        for (iter = treeDumpList; iter != NULL; iter = iter->next)
        {
            rect = (rect_t *)iter->payload.data;
            if (rect->image != NULL && rect->image->filename != NULL)
            {
                bitmap_t *png = Bitmap_CreateFromPNG(rect->image->filename);
                printf("        %d - %d %s\n", rect->x, rect->y, rect->image->filename);
                Bitmap_CopyPasteBitmap(canvas, png, rect->x, rect->y);
                Bitmap_Destroy(png);
            }
        }
        
        Bitmap_WriteAsPNG(canvas, "canvas.png");
        Bitmap_Destroy(canvas);
    }
    
    printf("-----\n");
    List_Destroy(treeDumpList, ListDestroy_List);
    
    if (pngFileList != NULL)
        List_Destroy(pngFileList, ListDestroy_List|ListDestroy_PayLoad);
    
    if (tree != NULL)
        BTree_Destroy(tree);
    
    memtrack_list_allocations();
}

int main(int argc, const char * argv[])
{
    param_t param;
    test_algorithm();
    Util_CompileRects(NULL, 0, "shit");
    /* packpng(); */
    
    Util_ParseParameters(&param, argc, argv);
    
    if (!param.valid)
        return 0;
    
    return 0;
}

