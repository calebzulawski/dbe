/*  DBE-CLI.C - The command line interface for the DataBending Editor
    Created by Caleb Zulawski
    More information about flags/options/configuration will come as this project develops further.
    This software is released as free software under the GNU General Public License v2. */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

/* The specifications for bitmap files were taken from http://en.wikipedia.org/wiki/BMP_file_format */

/* struct for BITMAP FILE HEADER */
#pragma pack(push, 1)
typedef struct tagBITMAPFILEHEADER
{
    uint16_t type;          //file type
    uint32_t size;          //file size in bytes
    uint16_t reserved1;     //reserved
    uint16_t reserved2;     //reserved
    uint32_t offset;        //offset in bytes from file header to image data
}BITMAPFILEHEADER;
#pragma pack(pop)

/* struct for BITMAP INFO HEADER */
#pragma pack(push, 1)
typedef struct tagBITMAPINFOHEADER
{
    uint32_t size;              //header size in bytes
    int32_t width;              //bitmap width in pixels
    int32_t height;             //bitmap height in pixels
    uint16_t planes;            //number of color planes (should be 1)
    uint16_t bitsperpx;         //number of bits per pixel
    uint32_t compression;       //compression method used (we wont be using this)
    uint32_t imageSize;         //image size in bytes
    int32_t xppm;               //horizontal resolution (pixels per meter)
    int32_t yppm;               //vertical resolution (pixels per meter)
    uint32_t colors;            //number of colors in color palette
    uint32_t colorsImportant;   //number of important colors (0 if every color is important)
}BITMAPINFOHEADER;
#pragma pack(pop)

/* function for reading the bitmap file */
unsigned char *LoadBitmapFile(char *filename, BITMAPFILEHEADER *bitmapFileHeader, BITMAPINFOHEADER *bitmapInfoHeader)
{
    FILE *fp;                   //bitmap file pointer
    unsigned char *imageData;   //image data will be stored here

    //open filename
    fp = fopen(filename,"rb");
    if (fp == NULL)
    {
        printf("Opening bitmap file failed.\n");
        return NULL;
    }
        

    //read BITMAPFILEHEADER into struct
    fread(bitmapFileHeader, sizeof(BITMAPFILEHEADER),1,fp);

    //verify file is BMP
    if (bitmapFileHeader->type != 0x4D42)
    {
        fclose(fp);
        printf("Selected file is not a BMP.\n");
        return NULL;
    }

    //read BITMAPINFOHEADER
    fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER),1,fp); // small edit. forgot to add the closing bracket at sizeof

    //move file pointer to offset of image as specified in the file header
    fseek(fp, bitmapFileHeader->offset, SEEK_SET);

    //allocate memory for the actual image data
    imageData = (unsigned char*)malloc(bitmapInfoHeader->imageSize);
    if (!imageData)
    {
        free(imageData);
        fclose(fp);
        printf("Memory allocation failed.\n");
        return NULL;
    }

    //read image data
    fread(imageData,bitmapInfoHeader->imageSize,1,fp);
    if (imageData == NULL)
    {
        fclose(fp);
        printf("Bitmap image data was not read successfully.\n");
        return NULL;
    }

    //return image
    fclose(fp);
    printf("Bitmap image read successfully.\n");
    return imageData;
}

/* function for writing bitmap files */
int WriteBitmapFile(char *filename, BITMAPFILEHEADER *bitmapFileHeader, BITMAPINFOHEADER *bitmapInfoHeader, unsigned char *imageData)
{
    FILE *fp; //bitmap file pointer

    //open filename
    fp = fopen(filename,"wb");
    if (fp == NULL)
    {
        printf("Creating bitmap file failed.\n");
        return -1;
    }

    //write file header, then info header
    fwrite(bitmapFileHeader,sizeof(BITMAPFILEHEADER),1,fp);
    fwrite(bitmapInfoHeader,sizeof(BITMAPINFOHEADER),1,fp);

    //move file pointer to offset in info header, then write the image data
    fseek(fp, bitmapFileHeader->offset, SEEK_SET);
    fwrite(imageData,bitmapInfoHeader->imageSize,1,fp);
    return 0;
}

/* function for flipping BGR to RGB or vice versa (bitmap uses BGR) */
unsigned char *reverseRGB(unsigned char *imageData, BITMAPINFOHEADER *bitmapInfoHeader)
{
    int idx = 0;            //image index var
    unsigned char tempRGB;  //temp var for RGB values

    for(idx = 0; idx < bitmapInfoHeader->imageSize; idx+=3)
    {
        tempRGB = imageData[idx];
        // swap R and B
        imageData[idx] = imageData[idx + 2];
        imageData[idx + 2] = tempRGB;
    }
    return imageData;
}

/* function for handling errors from lua */
void exitLua(lua_State *L, char *msg)
{
    //print error and display top of stack
	fprintf(stderr, "ERROR (Lua):\n%s: %s\n", msg, lua_tostring(L, -1));
	exit(1);
}
/* function for running Lua on the image data
   ARGUMENTS:
    returnData: pointer to the variable where the processed image will be dumped
    filename: filename of the Lua file
    bitmapData: the image data to be processed by Lua
    bytesPerPixel: number of bytes per pixel to be used when creating Lua array (can be adjusted for databending effects)
    bitmapLen: length of the bitmap array for the given value of bytesPerPixel
*/
void *runlua(uint64_t *returnData, char *filename, uint64_t *bitmapData, int bytesPerPixel, int bitmapLen)
{
    lua_State *L;

    L = luaL_newstate();    //create Lua state variable
    luaL_openlibs(L);       //open Lua libraries

    if (luaL_loadfile(L, filename))             //load Lua file (image effect)
    {
        exitLua(L, "luaL_loadfile() failed");   //exit if problems with file
    }         
	  
    printf("Pushing data to Lua.\n");
    //push image data to Lua as array
    lua_newtable(L);
    int i = 0;
    for(i = 0; i < bitmapLen; i++)
    {
        lua_pushnumber(L,i);
        lua_pushnumber(L,bitmapData[i]);
        lua_settable(L, -3);
    }
    lua_setglobal(L,"data");
    lua_pushnumber(L,bitmapLen);
    lua_setglobal(L,"dataLen");
    lua_pushnumber(L,8*bytesPerPixel);
    lua_setglobal(L,"dataBits");

    printf("Executing Lua...\n");
    //run the Lua file
    if (lua_pcall(L, 0, 0, 0))
    {
        exitLua(L, "lua_pcall() failed");
    }

    printf("Executing Lua complete.\n");

    //pull image data from Lua
    lua_getglobal(L,"data");
    for(i = 0; i < bitmapLen; i++)
    {
        lua_pushnumber(L,i);
        lua_gettable(L, -2);
        returnData[i] = lua_tonumber(L,-1);
        lua_pop(L,1);
    }

    //close lua and return
    lua_close(L);
    return 0;
}

/* main() */
int main(int argc, char *argv[])
{
    char infile[256];
    char outfile[256];
    char luafile[256];
    char buf[256];
    int count, i, j, index, startIndex, bytesPerPixel, xmin, xmax, ymin, ymax, length;
    
    //read config file
    FILE *fp;
    fp = fopen(argv[1],"rb");
    if (fp == NULL)
    {
        printf("Opening config file failed.\n");
        return 0;
    }

    //parse filenames from config file
    fgets(infile,sizeof(infile),fp);
    fgets(outfile,sizeof(infile),fp);
    fgets(luafile,sizeof(luafile),fp);

    //remove newlines from filenames, maybe there is a better way
    for(i = 0; i < 256; i++)
    {
        if(infile[i] == '\n')
            infile[i] = '\0';
        if(outfile[i] == '\n')
            outfile[i] = '\0';
        if(luafile[i] == '\n')
            luafile[i] = '\0';
    }

    //parse options from config file
    fgets(buf,sizeof(buf),fp);
    sscanf(buf, "%d", &bytesPerPixel);
    fgets(buf,sizeof(buf),fp);
    sscanf(buf, "%d %d %d %d",&xmin,&xmax,&ymin,&ymax);

    printf("Reading from %s.\nWriting to %s.\n",infile,outfile);

    //read bitmap
    BITMAPFILEHEADER bitmapFileHeader;
    BITMAPINFOHEADER bitmapInfoHeader;
    unsigned char *bitmapData;

    bitmapData = LoadBitmapFile(infile, &bitmapFileHeader, &bitmapInfoHeader);
    printf("Size is: %d x %d\n",bitmapInfoHeader.width,bitmapInfoHeader.height);


    //get section of bitmap to modify
    length = (xmax - xmin + 1)*(ymax - ymin + 1);
    uint64_t selectData[length];

    count = 0;
    for(i = xmin; i <= xmax; i++)
    {
        for(j = ymin; j <= ymax; j++)
        {
            startIndex = bytesPerPixel*((i-1) + (j-1)*bitmapInfoHeader.width);
            selectData[count] = 0;
            for(index = startIndex; index < startIndex + bytesPerPixel; index++)
            {
                selectData[count] = selectData[count] << 8;
                selectData[count] += bitmapData[index];
            }
            count++;
        }
    }

    //run Lua and get the modified image data
    uint64_t modifiedData[length];
    runlua(modifiedData,luafile,selectData,bytesPerPixel,length);

    //modify bitmap data
    count = 0;
    for(i = xmin; i <= xmax; i++)
    {
        for(j = ymin; j <= ymax; j++)
        {
            startIndex = bytesPerPixel*((i-1) + (j-1)*bitmapInfoHeader.width);
            selectData[count] = 0;
            for(index = startIndex; index < startIndex + bytesPerPixel; index++)
            {
                bitmapData[index] = (modifiedData[count] >> 8*(bytesPerPixel-1)) & 0xFF;
                modifiedData[count] = modifiedData[count] << 8;
            }
            count++;
        }
    }

    //write new bitmap file
    WriteBitmapFile(outfile, &bitmapFileHeader, &bitmapInfoHeader, bitmapData);
    return 0;
}