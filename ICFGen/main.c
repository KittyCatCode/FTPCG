//
//  main.c
//  ICFGen
//
//  Created by Gabriel Pulido on 3/30/17.
//  Gatoleft 🐈 2017 TRESGATOS Software Development. No rights reserved.
//

#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include "lodepng.h"

typedef struct StoredDirectoryEntry {
    struct dirent dirent;
    struct StoredDirectoryEntry* next;
} StoredDirectoryEntry;
typedef struct ColorEntry {
    char components[3];
    struct ColorEntry* next;
} ColorEntry;

void toUppercase(char* data,size_t len) {
    for(int i=0;i<len;i++) {
        if(data[i]>='a'&&data[i]<='z') {
            data[i]-='a'-'A';
        }
    }
}

int endsInPNG(const char* name,size_t len) {
    if(len<4)return 0;
    char* data = malloc(len+1);
    memcpy(data, name, len+1);
    data[len]=0;
    toUppercase(data, len);
    int toRet = strcmp(data+len-4, ".PNG");
    free(data);
    return toRet==0;
}

int comp(const void* neko1, const void* neko2) {
    return strcmp(((struct dirent*)neko1)->d_name, ((struct dirent*)neko2)->d_name);
}
int main(int argc, const char * argv[]) {
    printf("Intermediate Composite Format generator by TRESGATOS Software Development version 1.0!\n");
    if(argc!=3) {
        printf("Simple Syntax:\n");
        printf("icfgen folderPath outputFile.icf\n");
        printf("The folder should have png frames in alphabetical order.\n");
        printf("If you are using ffmpeg to get frames, use a fixed amount of digits (%%0digitsd) to make sure the frame-files are sorted correctly by icfgen.\n");
    } else {
        printf("Please wait, this might take a while...\n");
        const char* folderName = argv[1];
        const char* outputName = argv[2];
        DIR* folder = opendir(folderName);
        if(folder==NULL) {
            printf("There was a problem opening the directory %s.\n",folderName);
            return -1;
        } else {
            StoredDirectoryEntry* listStart = NULL;
            StoredDirectoryEntry* current = NULL;
            
            size_t numEntries = 0;
            struct dirent* entry;
            while((entry = readdir(folder))) {
                if(listStart==NULL) {
                    listStart = malloc(sizeof(StoredDirectoryEntry));
                    current = listStart;
                    listStart->next=NULL;
                    listStart->dirent=*entry;
                } else {
                    StoredDirectoryEntry* new = malloc(sizeof(StoredDirectoryEntry));
                    current->next=new;
                    current=new;
                    current->next=NULL;
                    current->dirent=*entry;
                }
                numEntries++;
            }
            closedir(folder);
            
            //now to put that into an array
            struct dirent* entryArray = malloc(sizeof(struct dirent)*numEntries);
            current=listStart;
            off_t i=0;
            do {
                memcpy(entryArray+i, &current->dirent, sizeof(struct dirent));
                i++;
            } while ((current=current->next));
            
            //free the linked list
            current=listStart;
            StoredDirectoryEntry* storedNext;
            do {
                storedNext=current->next;
                free(current);
            }while((current=storedNext));
            
            //sort the array alphabetically
            qsort(entryArray, numEntries, sizeof(struct dirent), comp);
            
            ColorEntry* colorListStart = NULL;
            ColorEntry* colorCurrent = NULL;
            
            int lastPercent = -1;
            for(int i=0;i<numEntries;i++) {
                if(((i*100)/numEntries)!=lastPercent) {
                    printf("%lu%% done\n",((i*100)/numEntries));
                    lastPercent=((i*100)/numEntries);
                }
                struct dirent* entry = entryArray+i;
                
                char* fileName = entry->d_name;
                size_t fileNameLen = strlen(fileName);
                
                if(!endsInPNG(fileName, fileNameLen))continue;
                
                size_t folderPathLen = strlen(folderName);
                int hasTrailingSlash = folderName[folderPathLen-1] == '/';
                
                size_t fullFilePathSize = folderPathLen+(hasTrailingSlash?0:1)+fileNameLen;
                char* fullFilePath = malloc(fullFilePathSize+1);
                
                memcpy(fullFilePath, folderName, folderPathLen);
                if(!hasTrailingSlash) {
                    fullFilePath[folderPathLen]='/';
                    memcpy(fullFilePath+folderPathLen+1, fileName, fileNameLen);
                } else {
                    memcpy(fullFilePath+folderPathLen, fileName, fileNameLen);
                }
                fullFilePath[fullFilePathSize]=0;
                
                unsigned char* image;
                unsigned width,height;
                unsigned error = lodepng_decode32_file(&image, &width, &height, fullFilePath);
                if(error) {
                    printf("Error loading file %s, skipping. Details: %s\n",fullFilePath,lodepng_error_text(error));
                } else {
                    unsigned long long components[3] = {0,0,0};
                    for(unsigned i=0;i<width*height;i++) {
                        components[0]+=image[i*4+0];
                        components[1]+=image[i*4+1];
                        components[2]+=image[i*4+2];
                    }
                    components[0]/=width*height;
                    components[1]/=width*height;
                    components[2]/=width*height;
                    if(colorListStart==NULL) {
                        colorListStart=malloc(sizeof(ColorEntry));
                        colorCurrent=colorListStart;
                        colorCurrent->next=NULL;
                        for(int i=0;i<3;i++) {
                            colorCurrent->components[i]=components[i];
                        }
                    } else {
                        ColorEntry* new = malloc(sizeof(ColorEntry));
                        colorCurrent->next=new;
                        colorCurrent=new;
                        colorCurrent->next=NULL;
                        for(int i=0;i<3;i++) {
                            colorCurrent->components[i]=components[i];
                        }
                    }
                }
                free(image);
                free(fullFilePath);
            }
            free(entryArray);
            //now to make that linked list of colors into a file
            FILE* output = fopen(outputName, "wb");
            if(output==NULL) {
                printf("Error opening output file for writing.\n");
                return -1;
            }
            colorCurrent=colorListStart;
            if(colorCurrent==NULL) {
                printf("Can't find images!\n");
                return -1;
            }
            ColorEntry* colorStoredNext;
            do {
                fwrite(colorCurrent->components, 1, 3, output);
                colorStoredNext=colorCurrent->next;
                free(colorCurrent);
            } while((colorCurrent=colorStoredNext));
            fclose(output);
            printf("All done!\n");
        }
    }
    return 0;
}
