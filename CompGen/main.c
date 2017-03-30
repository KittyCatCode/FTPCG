//
//  main.c
//  CompositeGen
//
//  Created by Gabriel Pulido on 3/30/17.
//  Gatoleft 🐈 2017 TRESGATOS Software Devopment. No rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include "lodepng.h"

int main(int argc, const char * argv[]) {
    printf("From-Intermediate Composite Generator by TRESGATOS Software Development version 1.0!\n");
    if(argc!=4) {
        printf("Simple Syntax:\n");
        printf("compgen input.icf width output.png\n");
    } else {
        unsigned int width = (unsigned int)strtol(argv[2],NULL,0);
        if(width) {
            FILE* icf = fopen(argv[1], "rb");
            if(icf==NULL) {
                printf("Failed to open ICF file %s.\n",argv[1]);
                return -1;
            }
            size_t length = lseek(fileno(icf), 0, SEEK_END)+1;
            rewind(icf);
            unsigned char* imageData = malloc(2*length);//bunch of padding for when length divided width is not an integer
            memset(imageData, 255, 2*length);
            fread(imageData, 1, length, icf);
            fclose(icf);
            size_t numPix = length/3;
            //now to make that an image
            unsigned error = lodepng_encode24_file(argv[3], imageData, width, (int)ceil((double)numPix/width));
            if(error) {
                printf("Error encoding PNG! Details: %s.\n",lodepng_error_text(error));
            }
        } else {
            printf("Width cannot be zero!\n");
        }
    }
    return 0;
}
