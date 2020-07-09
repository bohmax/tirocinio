//
//  h264toPng.c
//  ImagePicker
//
//  Created by Massimo Puddu on 03/04/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#include "h264toPng.h"

//AVCodecContext *pngContext = NULL;
//struct SwsContext* img_convert_ctx = NULL;

void logging(const char *fmt, ...){
    va_list args;
    fprintf( stderr, "LOG: " );
    va_start( args, fmt );
    vfprintf( stderr, fmt, args );
    va_end( args );
    fprintf( stderr, "\n" );
}

void plot_value(char path_r[], int gop_num, int FrameNo, int op, gop_info* info){
    int sum = 0, i = 0;
    char PNGNameSender[128];
    sprintf(PNGNameSender, "%sframe-%06d-%06d.png", path_image_sender, gop_num, FrameNo);
    if(op)
        path_r = PNGNameSender;
    pthread_mutex_lock(&mtxplot);
    fprintf(pipe_plot, "%s %s %d %d", path_r, PNGNameSender, gop_num, FrameNo);
    //vado all'indice da utilizzare di losted
    for(i=0; i<FrameNo ;i++)
        sum+=info->len_losted[i];
    for(i=0; i<info->len_losted[FrameNo] ;i++)
        fprintf(pipe_plot, " %d", info->losted_packet[sum+i]);
    fprintf(pipe_plot, "\n");
    fflush(pipe_plot);
    pthread_mutex_unlock(&mtxplot);
}

void savePNG(AVPacket* packet, char PNGName[]){
    FILE *PNGFile;
    if((PNGFile = fopen(PNGName, "wb"))){
        fwrite(packet->data, 1, packet->size, PNGFile);
        fclose(PNGFile);
    } else printf("Error: %s\n", strerror(errno));
}

int decode_to_png(AVFrame *pFrame, int FrameNo, int gop_num, gop_info* info) {
    int result = 0;
    char PNGName[128];
    // codec per png
    AVCodecContext *pngContext = NULL;
    AVCodec *pngCodec = avcodec_find_encoder(AV_CODEC_ID_PNG);
    if (!pngCodec) {
        exit(-1);
    }
    pngContext = avcodec_alloc_context3(pngCodec);
    if (!pngContext) {
        exit(-1);
    }
    
    pngContext->pix_fmt = AV_PIX_FMT_RGB24;
    pngContext->time_base = (AVRational) { .num = 25, .den = 1};
    pngContext->framerate = (AVRational) { .num = 0, .den = 0};
    pngContext->height = pFrame->height;
    pngContext->width = pFrame->width;
    //pngContext->thread_count = 0;
    //pngContext->thread_type = FF_THREAD_FRAME;
    if ((result = avcodec_open2(pngContext, pngCodec, NULL)) < 0) {
        printf("%s\n", av_err2str(result));
        exit(-1);
    }
    int response = avcodec_send_frame(pngContext, pFrame);
    if (response < 0) {
        logging("Error while sending a packet to the decoder: %s", av_err2str(response));
        return response;
    }
    AVPacket* packet = av_packet_alloc();
    if (!packet){
        logging("failed to allocated memory for AVPacket");
        return -1;
    }
    if (response >= 0) {
        response = avcodec_receive_packet(pngContext, packet);
        if (response == AVERROR(EAGAIN) || response == AVERROR_EOF){
            return (response==AVERROR(EAGAIN)) ? 0:1;
        }
        else if (response < 0) {
            fprintf(stderr, "Error during encoding\n");
            exit(1);
        }
        sprintf(PNGName, "%sframe-%06d-%06d.png", path_image, gop_num, FrameNo);
        savePNG(packet, PNGName);
        plot_value(PNGName, gop_num, FrameNo, 0, info);
        av_packet_unref(packet);
    }
    av_packet_free(&packet);
    avcodec_free_context(&pngContext);
    return 0;
}

int pixel_to_rgb24(AVFrame *pFrame,int pix_fmt ,int FrameNo, int gop_num, gop_info* info) {
    //pFrame=avcodec_alloc_frame();
    // Allocate an AVFrame structure
    struct SwsContext* img_convert_ctx = sws_getContext(pFrame->width, pFrame->height, pix_fmt, pFrame->width, pFrame->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
    if(!img_convert_ctx) {
        fprintf(stderr, "Cannot initialize the conversion context!\n");
        exit(1);
    }
    AVFrame* pFrameRGB=av_frame_alloc();
    if(pFrameRGB==NULL)
        return -1;
    
    int numBytes=avpicture_get_size(AV_PIX_FMT_RGB24, pFrame->width, pFrame->height);
    
    uint8_t* buffer = malloc(sizeof(uint8_t)*numBytes);
    
    // Assign appropriate parts of buffer to image planes in pFrameRGB
    avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24, pFrame->width, pFrame->height);

    int ret = sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pFrame->height, pFrameRGB->data, pFrameRGB->linesize);
    if(ret != pFrame->height) {
        fprintf(stderr, "SWS_Scale failed [%d]!\n", ret);
        exit(-1);
    }
    // Save the frame to disk
    pFrameRGB->format = pFrame->format;
    pFrameRGB->height = pFrame->height;
    pFrameRGB->width = pFrame->width;
    pFrameRGB->pts = FrameNo;
    //SaveFrame(pFrameRGB, gop_num, FrameNo); // se si volesse salvarli in formato ppm
    decode_to_png(pFrameRGB, FrameNo, gop_num, info);
    av_frame_free(&pFrameRGB);
    free(buffer);
    sws_freeContext(img_convert_ctx);
    return 0;
}

int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, int gop_num, gop_info* info){
    // https://ffmpeg.org/doxygen/trunk/structAVFrame.html
    AVFrame *pFrame = av_frame_alloc();
    if (!pFrame){
        logging("failed to allocated memory for AVFrame");
        return -1;
    }
    pCodecContext->time_base = (AVRational) {1, 50};
    // Supply raw packet data as input to a decoder
    // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga58bc4bf1e0ac59e27362597e467efff3
    int response = avcodec_send_packet(pCodecContext, pPacket);
    //output("nome");
    if (response < 0) {
        logging("Error while sending a packet to the decoder: %s", av_err2str(response));
        //send data to plot
        plot_value("", gop_num, pCodecContext->frame_number, 1, NULL);
        return response;
    }
    while (avcodec_receive_frame(pCodecContext, pFrame) >= 0 ){
        // Return decoded output data (into a frame) from a decoder
        // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga11e6542c4e66d3028668788a1a74217c
        logging(
                "Frame %d (type=%c, size=%d bytes) pts %d key_frame %d [DTS %d]",
                pCodecContext->frame_number,
                av_get_picture_type_char(pFrame->pict_type),
                pFrame->pkt_size,
                pFrame->pts,
                pFrame->key_frame,
                pFrame->coded_picture_number);
        //decode_to_png(pCodecContext, pFrame, pCodecContext->frame_number);
        pixel_to_rgb24(pFrame, pCodecContext->pix_fmt, pCodecContext->frame_number, gop_num, info);
        //pthread_mutex_lock(&mtx);
        //pushList(&testa, &coda, pFrame, pCodecContext->frame_number);
        //pthread_cond_signal(&cond);
        //pthread_mutex_unlock(&mtx);
        av_frame_unref(pFrame);
    }
    av_frame_free(&pFrame);
    return 0;
}

int create_image(gop_info* info){
    int result; //variabile per gli errori
    //pthread_t decode[NUMDECODERTHR];
    //printf("creando i thread\n");
    //for (int i = 0; i < NUMDECODERTHR; i++)
    //    SYSFREE(result,pthread_create(&decode[i],NULL,&decoder,NULL),0,"thread")
    //printf("thread creati...\n");
    AVFormatContext *c = NULL;
    char GOPName[64];
    sprintf(GOPName, "%sgop-%06d", path_file, info->gop_num);
    logging("decodificando il file %s\n", GOPName);
    result = avformat_open_input(&c, GOPName, NULL, NULL);
    if ( result != 0){
        logging("Cannot open file");
        printf("%s\n", av_err2str(result));
        return -1;
    }
    
    printf("format %s, duration %lld us, bit_rate %lld\n", c->iformat->name, c->duration, c->bit_rate);
    
    if (avformat_find_stream_info(c,  NULL) < 0) {
        logging("ERROR could not get the stream info");
        return -1;
    }
    
    // the component that knows how to enCOde and DECode the stream
    // it's the codec (audio or video)
    // http://ffmpeg.org/doxygen/trunk/structAVCodec.html
    AVCodec *pCodec = NULL;
    // this component describes the properties of a codec used by the stream i
    // https://ffmpeg.org/doxygen/trunk/structAVCodecParameters.html
    AVCodecParameters *pCodecParameters =  NULL;
    int video_stream_index = -1;
    
    for (int i = 0; i < c->nb_streams; i++){
        AVCodecParameters *pLocalCodecParameters =  NULL;
        pLocalCodecParameters = c->streams[i]->codecpar;
        logging("AVStream->time_base before open coded %d/%d", c->streams[i]->time_base.num, c->streams[i]->time_base.den);
        logging("AVStream->r_frame_rate before open coded %d/%d", c->streams[i]->r_frame_rate.num, c->streams[i]->r_frame_rate.den);
        logging("AVStream->start_time %" PRId64, c->streams[i]->start_time);
        logging("AVStream->duration %" PRId64, c->streams[i]->duration);
        
        logging("finding the proper decoder (CODEC)");
        
        AVCodec *pLocalCodec = NULL;
        
        // finds the registered decoder for a codec ID
        // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga19a0ca553277f019dd5b0fec6e1f9dca
        pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);
        
        if (pLocalCodec==NULL) {
            logging("ERROR unsupported codec!");
            return -1;
        }
        
        // when the stream is a video we store its index, codec parameters and codec
        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (video_stream_index == -1) {
                video_stream_index = i;
                pCodec = pLocalCodec;
                pCodecParameters = pLocalCodecParameters;
            }
            
            logging("Video Codec: resolution %d x %d", pLocalCodecParameters->width, pLocalCodecParameters->height);
        } else if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_AUDIO) {
            logging("Audio Codec: %d channels, sample rate %d", pLocalCodecParameters->channels, pLocalCodecParameters->sample_rate);
        }
        
        // print its name, id and bitrate
        logging("\tCodec %s ID %d bit_rate %lld", pLocalCodec->name, pLocalCodec->id, pLocalCodecParameters->bit_rate);
    }
    // https://ffmpeg.org/doxygen/trunk/structAVCodecContext.html
    AVCodecContext *pCodecContext = avcodec_alloc_context3(pCodec);
    if (!pCodecContext){
        logging("failed to allocated memory for AVCodecContext");
        return -1;
    }
    
    // Fill the codec context based on the values from the supplied codec parameters
    // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#gac7b282f51540ca7a99416a3ba6ee0d16
    if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0)
    {
        logging("failed to copy codec params to codec context");
        return -1;
    }
    
    // Initialize the AVCodecContext to use the given AVCodec.
    // https://ffmpeg.org/doxygen/trunk/group__lavc__core.html#ga11f785a188d7d9df71621001465b0f1d
    if (avcodec_open2(pCodecContext, pCodec, NULL) < 0){
        logging("failed to open codec through avcodec_open2");
        return -1;
    }
    
    //AVPacket **pPacket = NULL;
    //pPacket = malloc(sizeof(AVPacket*)*gop_info.num_frame);
    // https://ffmpeg.org/doxygen/trunk/structAVPacket.html
    //for (int i = 0; i < gop_info.num_frame; i++) {
    //    pPacket[i] = av_packet_alloc();
    //    if (!pPacket){
    //        logging("failed to allocated memory for AVPacket");
    //        return -1;
    //    }
    //}
    AVPacket* pPacket = av_packet_alloc();
    if (!pPacket){
        logging("failed to allocated memory for AVPacket");
        return -1;
    }

    int ret = 0;
    // fill the Packet with data from the Stream
    // https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga4fdb3084415a82e3810de6ee60e46a61
    while ((ret = av_read_frame(c, pPacket)) >= 0) {
        if (pPacket->stream_index == video_stream_index)
            decode_packet(pPacket, pCodecContext, info->gop_num, info);
        // https://ffmpeg.org/doxygen/trunk/group__lavc__packet.html#ga63d5a489b419bd5d45cfd09091cbcbc2
        if(pPacket)
            av_packet_unref(pPacket);
        
    }
    pthread_mutex_lock(&mtxplot);
    fprintf(pipe_plot, "Fine %d\n", info->gop_num);
    fflush(pipe_plot);
    pthread_mutex_unlock(&mtxplot);
    
    logging("releasing all the resources");
    
    avformat_close_input(&c);
    //for (int i = 0; i < gop_info.num_frame; i++)
    //av_packet_free(&pPacket[i]);
    av_packet_free(&pPacket);
    //free(pPacket);
    avcodec_free_context(&pCodecContext);
    return 0;
}

/* thread per il multi thread
void* decoder(void* arg){
    printf("Thread\n");
    AVFrame* frame = NULL;
    int nFrame = 0;
    list_dec* el = NULL;
    int fine = 0;
    while (!fine) {
        pthread_mutex_lock(&mtx);
        while (testa == NULL) {
            pthread_cond_wait(&cond, &mtx);
        }
        el = popListDec(&testa, &coda);
        pthread_mutex_unlock(&mtx);
        if (el->frame) {
            frame = el->frame;
            nFrame = el->Nframe;
            pixel_to_rgb24(frame, nFrame);
        } else fine = 1;
    }
    freeListDec(&el);
    return (void*)0;
}*/
