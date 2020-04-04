//
//  h264toPng.h
//  ImagePicker
//
//  Created by Massimo Puddu on 03/04/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#ifndef h264toPng_h
#define h264toPng_h

#include "utility.h"

void logging(const char *fmt, ...);

//salva un png in un file
void savePNG(AVPacket* packet, int FrameNo);

//MIA
//Converta in png
int decode_to_png(AVCodecContext *pCodecCtx, AVFrame *pFrame, int FrameNo);

//funzione simile a una trovata in rete
//codifica i pixel in rgb24
int decode_to_rgb24(AVCodecContext *pCodecCtx, AVFrame *pFrame, int FrameNo);

//sempre dal link sotto in parte
//prende ogni singolo frame o le prepara alla codifica in png
int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame);

//https://github.com/leandromoreira/ffmpeg-libav-tutorial/blob/master/0_hello_world.c
//decodifica un file h264 e crea AVPacket che contengono un frame
int create_image(void);

#endif /* h264toPng_h */
/*
 static void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame)
{
    FILE *pFile;
    char szFilename[64];
    int  y;
    
    // Open file
    sprintf(szFilename, "%sframe%d.ppm", path_file,iFrame);
    pFile=fopen(szFilename, "wb");
    if(pFile==NULL)
        return;
    
    // Write header
    fprintf(pFile, "P6\n%d %d\n255\n", width, height);
    
    // Write pixel data
    for(y=0; y<height; y++)
        fwrite(pFrame->data[0]+y*pFrame->linesize[0], 1, width*3, pFile);
    
    // Close file
    fclose(pFile);
}*/
