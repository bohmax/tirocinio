//
//  h264toPng.c
//  ImagePicker
//
//  Created by Massimo Puddu on 03/04/2020.
//  Copyright © 2020 Massimo Puddu. All rights reserved.
//

#include "h264toPng.h"

AVCodecContext *pngContext = NULL;

void logging(const char *fmt, ...){
    va_list args;
    fprintf( stderr, "LOG: " );
    va_start( args, fmt );
    vfprintf( stderr, fmt, args );
    va_end( args );
    fprintf( stderr, "\n" );
}

void savePNG(AVPacket* packet, int FrameNo){
    FILE *PNGFile;
    char PNGName[128];
    sprintf(PNGName, "%sframe-%06d.png", path_image, FrameNo);
    PNGFile = fopen(PNGName, "wb");
    fwrite(packet->data, 1, packet->size, PNGFile);
    fclose(PNGFile);
}

int decode_to_png(AVCodecContext *pCodecCtx, AVFrame *pFrame, int FrameNo) {
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
        
        savePNG(packet, FrameNo);
        av_packet_free(&packet);
    }
    return 0;
}

int pixel_to_rgb24(AVCodecContext *pCodecCtx, AVFrame *pFrame, int FrameNo) {
    //pFrame=avcodec_alloc_frame();
    // Allocate an AVFrame structure
    AVFrame* pFrameRGB=av_frame_alloc();
    if(pFrameRGB==NULL)
        return -1;
    
    int numBytes=avpicture_get_size(AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
    
    uint8_t* buffer = malloc(sizeof(uint8_t)*numBytes);
    
    // Assign appropriate parts of buffer to image planes in pFrameRGB
    avpicture_fill((AVPicture *)pFrameRGB, buffer, AV_PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
    
    struct SwsContext* img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_RGB24, SWS_BICUBIC, NULL, NULL, NULL);
    if(img_convert_ctx == NULL) {
        fprintf(stderr, "Cannot initialize the conversion context!\n");
        exit(1);
    }
    int ret = sws_scale(img_convert_ctx, pFrame->data, pFrame->linesize, 0, pFrame->height, pFrameRGB->data, pFrameRGB->linesize);
    if(ret != pFrame->height) {
        fprintf(stderr, "SWS_Scale failed [%d]!\n", ret);
        exit(-1);
    }
    // Save the frame to disk
    pFrameRGB->format = pFrame->format;
    pFrameRGB->height = pCodecCtx->height;
    pFrameRGB->width = pCodecCtx->width;
    decode_to_png(pCodecCtx, pFrameRGB, FrameNo);
    av_frame_free(&pFrameRGB);
    sws_freeContext(img_convert_ctx);
    free(buffer);
    //SaveFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height, FrameNo);
    return 0;
}

int decode_packet(AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame){
    // Supply raw packet data as input to a decoder
    // https://ffmpeg.org/doxygen/trunk/group__lavc__decoding.html#ga58bc4bf1e0ac59e27362597e467efff3
    int response = avcodec_send_packet(pCodecContext, pPacket);
    //output("nome");
    if (response < 0) {
        logging("Error while sending a packet to the decoder: %s", av_err2str(response));
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
        pixel_to_rgb24(pCodecContext, pFrame, pCodecContext->frame_number);
    }
    return 0;
}

int create_image(){
    int result; //variabile per gli errori
    AVFormatContext *c = NULL;
    printf("%s\n", path_file);
    result = avformat_open_input(&c, path_file, NULL, NULL);
    if ( result != 0){
        logging("Cannot open file");
        printf("%s\n", av_err2str(result));
        return -1;
    }
    
    printf("format %s, duration %lld us, bit_rate %lld", c->iformat->name, c->duration, c->bit_rate);
    
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
    
    // https://ffmpeg.org/doxygen/trunk/structAVFrame.html
    AVFrame *pFrame = av_frame_alloc();
    if (!pFrame){
        logging("failed to allocated memory for AVFrame");
        return -1;
    }
    // https://ffmpeg.org/doxygen/trunk/structAVPacket.html
    AVPacket *pPacket = av_packet_alloc();
    if (!pPacket){
        logging("failed to allocated memory for AVPacket");
        return -1;
    }
    
    // codec per png
    AVCodec *pngCodec = avcodec_find_encoder(AV_CODEC_ID_PNG);
    if (!pngCodec) {
        return -1;
    }
    pngContext = avcodec_alloc_context3(pngCodec);
    if (!pngContext) {
        return -1;
    }
    
    pngContext->pix_fmt = AV_PIX_FMT_RGB24;
    pngContext->time_base = (AVRational) { .num = 1, .den = 25};
    //pngContext->framerate = (AVRational) { .num = 1, .den = 25};
    pngContext->height = pCodecContext->height;
    pngContext->width = pCodecContext->width;
    if ((result = avcodec_open2(pngContext, pngCodec, NULL)) < 0) {
        printf("%s\n", av_err2str(result));
        return -1;
    }
    
    // fill the Packet with data from the Stream
    // https://ffmpeg.org/doxygen/trunk/group__lavf__decoding.html#ga4fdb3084415a82e3810de6ee60e46a61
    while (av_read_frame(c, pPacket) >= 0) {
        // if it's the video stream
        if (pPacket->stream_index == video_stream_index) {
            //logging("AVPacket->pts %" PRId64, pPacket->pts);
            decode_packet(pPacket, pCodecContext, pFrame);
            //output("/Users/maxuel/Desktop/img.png", pPacket);
            //if (response < 0)
            //  break;
        }
        // https://ffmpeg.org/doxygen/trunk/group__lavc__packet.html#ga63d5a489b419bd5d45cfd09091cbcbc2
        av_packet_unref(pPacket);
    }
    
    logging("releasing all the resources");
    
    avformat_close_input(&c);
    av_packet_free(&pPacket);
    av_frame_free(&pFrame);
    avcodec_free_context(&pCodecContext);
    avcodec_free_context(&pngContext);
    return 0;
}
