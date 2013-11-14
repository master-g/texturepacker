/*
 *  texturepacker.h
 *  texturepacker
 *
 *  Created by Master.G on 13-9-12.
 *  Copyright (c) 2013 Master.G. All rights reserved.
 */

#ifndef TEXTUREPACKER_H_
#define TEXTUREPACKER_H_

#define MEMORY_ERROR_MSG    "Insufficient memory\n"
#define GENERATE_STRING     "Generated by TextureAltasGenerator v1.0 on "
#define COPYRIGHT_STRING    "Author: Master.G"
#define ARGBSIZE            4

#define WRITE_STR_TO_FILE(str, fp) fwrite((str), 1, strlen(str), (fp))

/*
 * ************************************************************
 * image data structure
 * ************************************************************
 */

/**
 *	@brief	image data structure
 */
typedef struct image_t
{
	int w;          /**< width of the image, pixel */
	int h;          /**< height of the image, pixel */
	char *filename; /**< filename of the image */
    
} image_t;

#define IMAGE_AREA(image) (((image)->w) * ((image)->h))

/*
 * ************************************************************
 * rectangle data structure
 * ************************************************************
 */

/**
 *	@brief	rectangle
 */
typedef struct rect_t
{
	int x;          /**< the x offset of this rectangle in canvas */
	int y;          /**< the y offset of this rectangle in canvas */
	int w;          /**< width of this rectangle */
	int h;          /**< height of this rectangle */
    int padding;    /**< padding */
	image_t *image; /**< the image reference */
    
} rect_t;


/**
 *	@brief	print out a rect's info
 *
 *	@param 	rect 	the rect to print
 */
void Rect_Print(rect_t *rect);


/*
 * ************************************************************
 * list data structure
 * ************************************************************
 */

/**
 *	@brief	payload type enumeration
 */
typedef enum
{
	Payload_Image = 1,  /**< payload is a image data */
    Payload_Rect,       /**< payload is a rect */
	Payload_BTreeNode   /**< payload is a binary tree node */
    
} PayloadType;


/**
 *	@brief	list destroy mode enumeration
 */
typedef enum
{
	ListDestroy_PayLoad         = 1 << 0,   /**< free node and its payload data */
	ListDestroy_List            = 1 << 1    /**< free list */
    
} ListDestroyMode;

/**
 *	@brief	payload data structure
 */
typedef struct payload_t
{
	int type;   /**< type of this payload */
	void *data; /**< data */
    
} payload_t;


/**
 *	@brief	list data structure
 */
typedef struct list_t
{
	payload_t payload;      /**< payload */
	struct list_t *next;    /**< link */
    
} list_t;

/* ************************************************************ */

/**
 *	@brief	create a empty list node
 *
 *	@return list_t*
 */
list_t *List_Create(void);

/**
 *	@brief	free a list/node with parameter
 *
 *	@param 	list 	the list/node to free
 *	@param 	mode 	how to free the list/node
 */
void List_Destroy(list_t *list, int mode);

/**
 *	@brief	push a list/node to the back of list
 *
 *	@param 	list 	the list to push into
 *	@param 	node 	the list/node to push
 */
void List_PushBack(list_t *list, list_t *node);

/**
 *	@brief	push a list/node to the front of list
 *
 *	@param 	*head 	the ref of the list to push into
 *	@param 	node 	the list/node to push
 */
void List_PushFront(list_t **head, list_t *node);

/**
 *	@brief	get tail of a list/node
 *
 *	@param 	list
 *
 *	@return	tail
 */
list_t *List_GetTail(list_t *list);

/**
 *	@brief	print a list
 *
 *	@param 	list 	the list to print
 */
void List_Print(list_t *list);

/**
 *	@brief	sort the list
 *
 *	@param 	list 	the payload of the list must be image or rect
 */
void List_Sort(list_t **list);

/*
 * ************************************************************
 * binary tree data structure
 * ************************************************************
 */

/**
 *	@brief	binary tree traverse mode
 */
typedef enum
{
	BTreeTraverse_InOrder = 0,  /**< in order */
	BTreeTraverse_PreOrder,     /**< pre order */
	BTreeTraverse_PostOrder     /**< post order */
    
} BTreeTraverseMode;

/**
 *	@brief	binary tree data structure
 */
typedef struct btree_t
{
	rect_t rect;            /**< rect */
	struct btree_t *left;   /**< link to left child */
	struct btree_t *right;  /**< link to right child */
    
} btree_t;

/**
 *	@brief	binary tree insertion result
 */
typedef enum
{
	Insert_OK = 0,  /**< insert success */
	Insert_Failed   /**< insert failed */
    
} BTreeInsertResult;

/* ************************************************************ */

/**
 *	@brief	binary tree traverse node visiting function pointer
 */
typedef void (*btree_visit_func)(btree_t *node, void *context);

/**
 *	@brief	create a empty binary tree node
 *
 *	@return	the new binary tree node
 */
btree_t *BTree_Create(void);

/**
 *	@brief	destroy a binary tree and all its leaves including payload
 *
 *	@param 	tree 	btree_t
 */
void BTree_Destroy(btree_t *tree);

/**
 *	@brief	traverse a binary tree
 *
 *	@param 	node 	the tree to traverse
 *	@param 	mode 	the traverse mode
 *	@param 	func 	node visiting function pointer
 *	@param 	context 	context will be pass to visiting function
 */
void BTree_Traverse(btree_t *tree, int mode, btree_visit_func func, void *context);

/**
 *	@brief	dump the tree to list
 *
 *	@param 	tree 	the tree to dump
 *	@param 	mode 	the traverse mode
 *
 *	@return	the list
 */
list_t *BTree_DumpToList(btree_t *tree, int mode);

/**
 *	@brief	try to insert a rectangle into binary tree
 *
 *	@param 	root 	the root to insert the image
 *	@param 	image 	the image to insert
 *  @param  padding the padding for image
 *
 *	@return	NULL for failed
 */
btree_t *BTree_Insert(btree_t *root, image_t *image, int padding);

/*
 * ************************************************************
 * utilities
 * ************************************************************
 */

typedef struct param_t
{
    int autoextend;
    int pot;
    int width;
    int height;
    int square;
    int format;
    int verbose;
    int valid;
    
    char *outfile;
    char *inpath;
    
} param_t;

/**
 *	@brief	output file format enumeration
 */
typedef enum
{
	FileFormat_Lua = 0, /**< lua format */
	FileFormat_Count    /**< total file format count */
} FileFormat;

/**
 *	@brief	bitmap structure
 */
typedef struct bitmap_t
{
	int w; /**< width of bitmap, in pixel */
	int h; /**< height of bitmap, in pixel */
	unsigned char *pixels; /**< pixel data */
} bitmap_t;

/* create a clean bitmap */
bitmap_t *Bitmap_Create(int w, int h);

/* create a bitmap from png file */
bitmap_t *Bitmap_CreateFromPNG(const char *filename);

/* destroy a bitmap */
void Bitmap_Destroy(bitmap_t *bitmap);

/* clean a bitmap with specific color */
void Bitmap_Clean(bitmap_t *bitmap, int argb);

/* draw a pixel to bitmap */
void Bitmap_DrawPixel(bitmap_t *bitmap, int x, int y, int rgba);

/* draw a bitmap on another bitmap */
void Bitmap_CopyPasteBitmap(bitmap_t *canvas, bitmap_t *bitmap, int x, int y);

/* write a bitmap to png file */
void Bitmap_WriteAsPNG(bitmap_t *bitmap, const char *filename);

/* find next POT of x */
int Util_NextPOT(int x);

/* read a png file's graphical information */
void Util_ReadPNGinfo(const char *filename, int *w, int *h);

/* extract extension from filename */
const char *Util_GetFileExtension(const char *filename);

/* copy string */
const char *Util_CopyString(char **dst, const char *src);

/* push a image file info to the back of the list */
void Util_PushFileToList(list_t **list, const char *filename);

/* load up files */
list_t *Util_ReadAllPNGs(void);

/* format date string */
void Util_GetDateString(char *buf, int *length);

/* compile rect list to file */
void Util_CompileRects(list_t *list, int format, const char* filename);

/* print simple usage message */
void Util_PrintSimpleUsage(void);

/* print exhaustive message */
void Util_PrintExhaustiveUsage(void);

/* parse parameters */
void Util_ParseParameters(param_t *param, int argc, const char *argv[]);

#endif /* TEXTUREPACKER_H_ */