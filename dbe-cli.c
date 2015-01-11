#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#pragma pack(push, 1)

typedef struct tagBITMAPFILEHEADER
{
    uint16_t bfType;  //specifies the file type
    uint32_t bfSize;  //specifies the size in bytes of the bitmap file
    uint16_t bfReserved1;  //reserved; must be 0
    uint16_t bfReserved2;  //reserved; must be 0
    uint32_t bfOffBits;  //species the offset in bytes from the bitmapfileheader to the bitmap bits
}BITMAPFILEHEADER;

#pragma pack(pop)

#pragma pack(push, 1)

typedef struct tagBITMAPINFOHEADER
{
    uint32_t biSize;  //specifies the number of bytes required by the struct
    uint32_t biWidth;  //specifies width in pixels
    uint32_t biHeight;  //species height in pixels
    uint16_t biPlanes; //specifies the number of color planes, must be 1
    uint16_t biBitCount; //specifies the number of bit per pixel
    uint32_t biCompression;//spcifies the type of compression
    uint32_t biSizeImage;  //size of image in bytes
    uint32_t biXPelsPerMeter;  //number of pixels per meter in x axis
    uint32_t biYPelsPerMeter;  //number of pixels per meter in y axis
    uint32_t biClrUsed;  //number of colors used by th ebitmap
    uint32_t biClrImportant;  //number of colors that are important
}BITMAPINFOHEADER;

#pragma pack(pop)

unsigned char *LoadBitmapFile(char *filename, BITMAPFILEHEADER *bitmapFileHeader, BITMAPINFOHEADER *bitmapInfoHeader)
{
    FILE *filePtr; //our file pointer
    unsigned char *bitmapImage;  //store image data

    //open filename in read binary mode
    filePtr = fopen(filename,"rb");
    if (filePtr == NULL)
    {
        printf("Opening bitmap file failed.\n");
        return NULL;
    }
        

    //read the bitmap file header
    fread(bitmapFileHeader, sizeof(BITMAPFILEHEADER),1,filePtr);

    //verify that this is a bmp file by check bitmap id
    if (bitmapFileHeader->bfType != 0x4D42)
    {
        fclose(filePtr);
        printf("Selected file is not a BMP.\n");
        return NULL;
    }

    //read the bitmap info header
    fread(bitmapInfoHeader, sizeof(BITMAPINFOHEADER),1,filePtr); // small edit. forgot to add the closing bracket at sizeof

    //move file point to the begging of bitmap data
    fseek(filePtr, bitmapFileHeader->bfOffBits, SEEK_SET);

    //allocate enough memory for the bitmap image data
    bitmapImage = (unsigned char*)malloc(bitmapInfoHeader->biSizeImage);

    //verify memory allocation
    if (!bitmapImage)
    {
        free(bitmapImage);
        fclose(filePtr);
        printf("Memory allocation failed.\n");
        return NULL;
    }

    //read in the bitmap image data
    fread(bitmapImage,bitmapInfoHeader->biSizeImage,1,filePtr);

    //make sure bitmap image data was read
    if (bitmapImage == NULL)
    {
        fclose(filePtr);
        printf("Bitmap image data was not read successfully.\n");
        return NULL;
    }

    //close file and return bitmap iamge data
    fclose(filePtr);
    printf("Bitmap image read successfully.\n");
    return bitmapImage;
}

int WriteBitmapFile(char *filename, BITMAPFILEHEADER *bitmapFileHeader, BITMAPINFOHEADER *bitmapInfoHeader, unsigned char *bitmapImage)
{
    FILE *filePtr; //our file pointer

    //open filename in read binary mode
    filePtr = fopen(filename,"wb");
    if (filePtr == NULL)
    {
        printf("Creating bitmap file failed.\n");
        return -1;
    }

    fwrite(bitmapFileHeader,sizeof(BITMAPFILEHEADER),1,filePtr);
    fwrite(bitmapInfoHeader,sizeof(BITMAPINFOHEADER),1,filePtr);
    fseek(filePtr, bitmapFileHeader->bfOffBits, SEEK_SET);
    fwrite(bitmapImage,bitmapInfoHeader->biSizeImage,1,filePtr);
    return 0;
}

unsigned char *reverseRGB(unsigned char *bitmapImage, BITMAPINFOHEADER *bitmapInfoHeader)
{
    int imageIdx=0;  //image index counter
    unsigned char tempRGB;  //our swap variable
    //swap the r and b values to get RGB (bitmap is BGR)
    for (imageIdx = 0;imageIdx < bitmapInfoHeader->biSizeImage;imageIdx+=3)
    {
        tempRGB = bitmapImage[imageIdx];
        bitmapImage[imageIdx] = bitmapImage[imageIdx + 2];
        bitmapImage[imageIdx + 2] = tempRGB;
    }
    return bitmapImage;
}

void bail(lua_State *L, char *msg){
	fprintf(stderr, "\nFATAL ERROR:\n  %s: %s\n\n",
		msg, lua_tostring(L, -1));
	exit(1);
}

void *runlua(uint64_t *returnData, char *filename, uint64_t *bitmapData, int bytesPerPixel, int bitmapLen)
{
    lua_State *L;

    L = luaL_newstate();                        /* Create Lua state variable */
    luaL_openlibs(L);                           /* Load Lua libraries */

    if (luaL_loadfile(L, filename))    /* Load but don't run the Lua script */
	bail(L, "luaL_loadfile() failed");      /* Error out if file can't be read */

    printf("Pushing data to Lua.\n");
    lua_newtable(L);
    int i = 0;
    printf("%d",bitmapLen);
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

    if (lua_pcall(L, 0, 0, 0))                  /* Run the loaded Lua script */
	bail(L, "lua_pcall() failed");          /* Error out if Lua file has an error */

    printf("Lua complete.\n");

    lua_getglobal(L,"data");
    for(i = 0; i < bitmapLen; i++)
    {
        lua_pushnumber(L,i);
        lua_gettable(L, -2);
        returnData[i] = lua_tonumber(L,-1);
        lua_pop(L,1);
    }
    lua_close(L);                               /* Clean up, free the Lua state var */

    return returnData;
}

int main(int argc, char *argv[])
{
    char infile[256];
    char outfile[256];
    char luafile[256];
    char buf[256];
    int count, i, j, index, startIndex, bytesPerPixel, xmin, xmax, ymin, ymax, length;
    //read config file
    FILE *filePtr; //our file pointer
    //open filename in read binary mode
    filePtr = fopen(argv[1],"rb");
    if (filePtr == NULL)
    {
        printf("Opening config file failed.\n");
        return 0;
    }
    fgets(infile,sizeof(infile),filePtr);
    fgets(outfile,sizeof(infile),filePtr);
    fgets(luafile,sizeof(luafile),filePtr);

    //remove newlines from filenames
    for(i = 0; i < 256; i++)
    {
        if(infile[i] == '\n')
            infile[i] = '\0';
        if(outfile[i] == '\n')
            outfile[i] = '\0';
        if(luafile[i] == '\n')
            luafile[i] = '\0';
    }

    fgets(buf,sizeof(buf),filePtr);
    sscanf(buf, "%d", &bytesPerPixel);
    fgets(buf,sizeof(buf),filePtr);
    sscanf(buf, "%d %d %d %d",&xmin,&xmax,&ymin,&ymax);

    printf("Reading from %s.\nWriting to %s.\n",infile,outfile);

    //read bitmap
    BITMAPFILEHEADER bitmapFileHeader;
    BITMAPINFOHEADER bitmapInfoHeader;
    unsigned char *bitmapData;

    bitmapData = LoadBitmapFile(infile, &bitmapFileHeader, &bitmapInfoHeader);
    printf("Size is: %d x %d\n",bitmapInfoHeader.biWidth,bitmapInfoHeader.biHeight);


    //get section of bitmap to modify
    length = (xmax - xmin + 1)*(ymax - ymin + 1);
    uint64_t selectData[length];

    count = 0;
    for(i = xmin; i <= xmax; i++)
    {
        for(j = ymin; j <= ymax; j++)
        {
            startIndex = bytesPerPixel*((i-1) + (j-1)*bitmapInfoHeader.biWidth);
            selectData[count] = 0;
            for(index = startIndex; index < startIndex + bytesPerPixel; index++)
            {
                selectData[count] = selectData[count] << 8;
                selectData[count] += bitmapData[index];
            }
            count++;
        }
    }

    //lua
    uint64_t modifiedData[length];
    runlua(modifiedData,luafile,selectData,bytesPerPixel,length);

    //modify bitmap data
    count = 0;
    for(i = xmin; i <= xmax; i++)
    {
        for(j = ymin; j <= ymax; j++)
        {
            startIndex = bytesPerPixel*((i-1) + (j-1)*bitmapInfoHeader.biWidth);
            selectData[count] = 0;
            for(index = startIndex; index < startIndex + bytesPerPixel; index++)
            {
                bitmapData[index] = (modifiedData[count] >> 8*(bytesPerPixel-1)) & 0xFF;
                modifiedData[count] = modifiedData[count] << 8;
            }
            count++;
        }
    }

    //write bitmap
    WriteBitmapFile(outfile, &bitmapFileHeader, &bitmapInfoHeader, bitmapData);
    return 0;
}