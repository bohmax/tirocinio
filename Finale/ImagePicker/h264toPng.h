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

//calcola PSNR e plotta il risultato, op indica se devo plottare un frame che sono stati decodifica (1) o meno (0)
void plot_value(char path_send[], int gop_num, int FrameNo, int op);

//salva un png in un file
void savePNG(AVPacket* packet, char PNGName[]);

//MIA
//Converta in png
int decode_to_png(AVFrame *pFrame, int FrameNo, int gop_num);

//funzione simile a una trovata in rete
//codifica i pixel in rgb24
int decode_to_rgb24(AVFrame *pFrame, int pix_fmt, int FrameNo, int gop_num);

//sempre dal link sotto in parte
//prende ogni singolo frame o le prepara alla codifica in png
int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, int gop_num);

//https://github.com/leandromoreira/ffmpeg-libav-tutorial/blob/master/0_hello_world.c
//decodifica un file h264 e crea AVPacket che contengono un frame
int create_image(gop_info* info);

//thread che gestisce la decodifica dell'immagine
void* decoder(void* arg);
#endif /* h264toPng_h */

/*void SaveFrame(AVFrame *pFrame, int numGop, int numFrame){
    FILE *pFile;
    int width = pFrame->width, height = pFrame->height;
    char PNGName[128];
    int  y;
    
    // Open file
    sprintf(PNGName, "%s-%06d-%06d.ppm", path_image, numGop, numFrame);
    pFile=fopen(PNGName, "wb");
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
