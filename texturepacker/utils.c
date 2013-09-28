/*
 *  utils.c
 *  texturepacker
 *
 *  Created by Master.G on 13-9-17.
 *  Copyright (c) 2013 Master.G. All rights reserved.
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "memtracker.h"
#include "texturepacker.h"
#include "lodepng.h"
#include "tinydir.h"

/* create a empty bitmap */
bitmap_t *Bitmap_Create(int w, int h)
{
    bitmap_t *bitmap = NULL;
    bitmap = (bitmap_t *)malloc(sizeof(bitmap_t));
    if (bitmap == NULL)
        printf(MEMORY_ERROR_MSG);
    
    bitmap->w = w;
    bitmap->h = h;
    bitmap->pixels = (unsigned char *)calloc(1, w * h * ARGBSIZE);
    
    if (bitmap->pixels == NULL)
        printf(MEMORY_ERROR_MSG);
    
    return bitmap;
}

/* create a bitmap from png file */
bitmap_t *Bitmap_CreateFromPNG(const char *filename)
{
    unsigned error;
    bitmap_t *bitmap = NULL;
    
    bitmap = (bitmap_t *)calloc(1, sizeof(bitmap_t));
    
	error = lodepng_decode32_file(&bitmap->pixels, (unsigned *)&bitmap->w, (unsigned *)&bitmap->h, filename);
	if(error)
        printf("error %u: %s\n", error, lodepng_error_text(error));
    
    return bitmap;
}

void Bitmap_Destroy(bitmap_t *bitmap)
{
    if (bitmap->pixels != NULL)
        free(bitmap->pixels);
    
    free(bitmap);
}

/* clean a bitmap with specific color */
void Bitmap_Clean(bitmap_t *bitmap, int argb)
{
    int i = 0;
    for (i = 0; i < bitmap->w * bitmap->h; i++)
        ((int *)bitmap->pixels)[i] = argb;
}

void Bitmap_DrawPixel(bitmap_t *bitmap, int x, int y, int rgba)
{
    unsigned char color[4];
    memcpy(color, &rgba, ARGBSIZE);
    bitmap->pixels[(x + bitmap->w * y) * ARGBSIZE + 0] = color[0];
    bitmap->pixels[(x + bitmap->w * y) * ARGBSIZE + 1] = color[1];
    bitmap->pixels[(x + bitmap->w * y) * ARGBSIZE + 2] = color[2];
    bitmap->pixels[(x + bitmap->w * y) * ARGBSIZE + 3] = color[3];
}

void Bitmap_CopyPasteBitmap(bitmap_t *canvas, bitmap_t *bitmap, int x, int y)
{
    int i = 0;
    int lineSize = 0;
    
    lineSize = bitmap->w * ARGBSIZE;
    
    for (i = 0; i < bitmap->h; i++)
        memcpy(&(canvas->pixels[(x + (y + i) * canvas->w) * ARGBSIZE]), &(bitmap->pixels[i * lineSize]), lineSize);
}

void Bitmap_WriteAsPNG(bitmap_t *bitmap, const char *filename)
{
    /*Encode the image*/
	unsigned error = lodepng_encode32_file(filename, bitmap->pixels, bitmap->w, bitmap->h);
    
	/*if there's an error, display it*/
	if(error) printf("error %u: %s\n", error, lodepng_error_text(error));
}

/* find next POT of x */
int Util_NextPOT(int x)
{
    x = x - 1;
    x = x | (x >> 1);
    x = x | (x >> 2);
    x = x | (x >> 4);
    x = x | (x >> 8);
    x = x | (x >>16);
    
    return x + 1;
}

/* print out a rect_t */
void Rect_Print(rect_t *rect)
{
    printf("Rect:%p x:%5d y:%5d w:%5d h:%5d image:%s\n",
           (void*)rect,
           rect->x,
           rect->y,
           rect->h,
           rect->w,
           (rect->image != NULL && rect->image->filename != NULL) ? rect->image->filename : "null");
}

/* read a png file's graphical information */
void Util_ReadPNGinfo(const char *filename, int *w, int *h)
{
    unsigned error;
    unsigned char* image;
    
    error = lodepng_decode32_file(&image, (unsigned *)w, (unsigned *)h, filename);
    if (error)
        printf("error %u: %s\n", error, lodepng_error_text(error));
    
    free(image);
}

/* extract extension from filename */
const char *Util_GetFileExtension(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    
    if(!dot || dot == filename)
        return "";
    
    return dot + 1;
}

/* copy string */
const char *Util_CopyString(char **dst, const char *src)
{
    size_t length = 0;
    if (*dst != NULL)
        free(*dst);
    
    length = strlen(src);
    
    *dst = calloc(1, length + 1);
    memcpy(*dst, src, length);
    
    return src;
}

/* push a image file info to the back of the list */
void Util_PushFileToList(list_t **list, const char *filename)
{
    image_t *img;
    list_t *node = NULL;
    
    if (strcmp(Util_GetFileExtension(filename), "png") != 0)
        return;
    
    img = (image_t *)malloc(sizeof(image_t));
    Util_CopyString(&img->filename, filename);
    
    Util_ReadPNGinfo(filename, &img->w, &img->h);
    
    if (*list == NULL)
    {
        (*list) = List_Create();
        (*list)->payload.type = Payload_Image;
        (*list)->payload.data = img;
    }
    else
    {
        node = List_Create();
        node->payload.type = Payload_Image;
        node->payload.data = img;
        List_PushBack(*list, node);
    }
}

/* load up files */
list_t *Util_ReadAllPNGs(void)
{
    list_t *filelist = NULL;
    tinydir_dir dir;
    int i = 0;
    
    tinydir_open_sorted(&dir, ".");
    
    for (i = 0; i < dir.n_files; i++)
    {
        tinydir_file file;
        tinydir_readfile_n(&dir, &file, i);
        
        if (!file.is_dir)
            Util_PushFileToList(&filelist, file.name);
    }
    
    tinydir_close(&dir);
    
    List_Sort(&filelist);
    
    return filelist;
}

#define DATELEN 20

/* format date string */
void Util_GetDateString(char *buf, int *length)
{
    struct tm *myTime = NULL;
    char chrDate[DATELEN];
    time_t mytm;
    
    time(&mytm);
    myTime = localtime(&mytm);
    strftime(chrDate, DATELEN, "%Y/%m/%d %H:%M:%S", myTime);
    
    if (length != NULL)
        *length = DATELEN;
    
    if (buf != NULL)
        memcpy(buf, chrDate, DATELEN);
    
    /* printf("[%s]\n", chrDate); */
}

/* header generator for lua */
void Lua_BegGenerator(const char* filename, FILE **fp)
{
    char *fnamebuf = NULL;
    char datestr[DATELEN];
    
    fnamebuf = calloc(1, strlen(filename) + 4);
    sprintf(fnamebuf, "%s.lua", filename);
    
    *fp = fopen(fnamebuf, "wb");
    
    WRITE_STR_TO_FILE("--\n-- ", *fp);
    WRITE_STR_TO_FILE(filename, *fp);
    WRITE_STR_TO_FILE(".lua\n--\n-- ", *fp);
    WRITE_STR_TO_FILE(GENERATE_STRING, *fp);
    
    Util_GetDateString(datestr, NULL);
    
    WRITE_STR_TO_FILE(datestr, *fp);
    WRITE_STR_TO_FILE("\n-- ", *fp);
    WRITE_STR_TO_FILE(COPYRIGHT_STRING, *fp);
    WRITE_STR_TO_FILE("\n--\n\n", *fp);
}

/* rectangle description generator for lua */
void Lua_CplGenerator(rect_t *rect, FILE *fp)
{
    image_t *img = NULL;
    
    img = (image_t *)rect->image;
    
}

/* end of file generator for lua */
void Lua_EndGenerator(FILE *fp)
{
    fclose(fp);
}

typedef void (*beg_func)(const char* filename, FILE **fp);
beg_func begfuncmap[FileFormat_Count];

typedef void (*cpl_func)(rect_t *rect, FILE *fp);
cpl_func cplfuncmap[FileFormat_Count];

typedef void (*end_func)(FILE *fp);
end_func endfuncmap[FileFormat_Count];

/* compile rect list to file */
void Util_CompileRects(list_t *list, int format, const char* filename)
{
    FILE *fp = NULL;
    list_t *iter = NULL;
    rect_t *rect = NULL;
    
    /* setting up function map */
    begfuncmap[FileFormat_Lua] = Lua_BegGenerator;
    cplfuncmap[FileFormat_Lua] = Lua_CplGenerator;
    endfuncmap[FileFormat_Lua] = Lua_EndGenerator;
    
    iter = list;
    
    begfuncmap[format](filename, &fp);
    
    for (iter = list; iter != NULL; iter = iter->next)
    {
        rect = (rect_t *)iter->payload.data;
        cplfuncmap[format](rect, fp);
    }
    
    endfuncmap[format](fp);
}

/* print simple usage message */
void Util_PrintSimpleUsage(void)
{
    printf("Usage:\n\n");
    printf("\ttextureatlas [options] <path> -o <filename>\n\n");
    printf("where <path> is the directory of the png files.\n");
    printf("Default is the directory where the textureatlas is.\n\n");
    printf("Try -help for an exhaustive list of advanced options.\n");
}

/* print exhaustive message */
void Util_PrintExhaustiveUsage(void)
{
    printf("Usage:\n");
    printf(" textureatlas [options] path [-o out_file]\n\n");
    printf("textureatlas can only accept png images\n");
    printf("options:\n");
    printf("  -h / -help ............. short help\n");
    printf("  -H / -longhelp ......... long help\n");
    printf("  -a <int> ............... auto resize the output texture (0: non POT ... 1: POT)\n");
    printf("  -wh <int> <int> ........ output texture width and height\n");
    printf("  -s <int> ............... output texture square width\n");
    printf("  -f <string> ............ output format, one of:\n");
    printf("                             lua\n");
}

/* parse parameters */
void Util_ParseParameters(param_t *param, int argc, const char *argv[])
{
    int c = 0;
    memset(param, 0, sizeof(param_t));
    
    /* parse parameters */
    if (argc == 1)
    {
        Util_PrintSimpleUsage();
        return;
    }
    
    for (c = 1; c < argc; c++)
    {
        if (!strcmp(argv[c], "-h") || !strcmp(argv[c], "-help"))
        {
            Util_PrintSimpleUsage();
            return;
        }
        else if (!strcmp(argv[c], "-H") || !strcmp(argv[c], "-longhelp"))
        {
            Util_PrintExhaustiveUsage();
            return;
        }
        else if (!strcmp(argv[c], "-a") && c < argc - 1)
        {
            param->autoextend = (int)strtol(argv[++c], NULL, 0);
            if (param->autoextend != 0 || param->autoextend != 1)
            {
                printf("Error! -a accept 0 or 1 only\n");
                break;
            }
        }
        else if (!strcmp(argv[c], "-wh") && c < argc - 2)
        {
            param->width = (int)strtol(argv[++c], NULL, 0);
            param->height = (int)strtol(argv[++c], NULL, 0);
            if (param->width <= 0 || param->height <= 0)
            {
                printf("Error! TextureAtlas only takes positive parameters\n");
                break;
            }
        }
        else if (!strcmp(argv[c], "-s") && c < argc - 1)
        {
            param->square = (int)strtol(argv[++c], NULL, 0);
        }
        else if (!strcmp(argv[c], "-f") && c < argc - 1)
        {
            if (!strcmp(argv[++c], "lua"))
            {
                param->format = FileFormat_Lua;
            }
            else
            {
                printf("Error! Unrecognized output format %s\n", argv[c]);
            }
        }
        else if (!strcmp(argv[c], "-o") && c < argc - 1)
        {
            Util_CopyString(&param->outfile, argv[++c]);
        }
        else if (!strcmp(argv[c], "-v"))
        {
            param->verbose = 1;
        }
        else if (argv[c][0] == '-')
        {
            printf("Error! Unknown options '%s'\n", argv[c]);
            Util_PrintExhaustiveUsage();
            break;
        }
        else
        {
            Util_CopyString(&param->inpath, argv[c]);
        }
    }
    
    /* validate parameter */
    if (param->outfile == NULL)
    {
        Util_PrintSimpleUsage();
        return;
    }
    
    if (param->outfile == NULL)
    {
        printf("Error! Output file missing\n");
        return;
    }
    
    if (param->inpath == NULL)
    {
        Util_CopyString(&param->inpath, ".");
        return;
    }
    
    if (param->square != 0 && (param->width + param->height) != 0)
    {
        printf("Warning! Both -s and -wh are set, textureatlas will use -s only\n");
        param->valid = 1;
        return;
    }
    else if (param->square != 0)
    {
        param->valid = 1;
        return;
    }
}
