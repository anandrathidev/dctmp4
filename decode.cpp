/**
 * Show how to use the libavformat and libavcodec API to demux and decode audio
 * and video data. Write the output as raw audio and input files to be played by
 * ffplay.
 */

 extern "C" {
    #include <libavutil/imgutils.h>
    #include <libavutil/motion_vector.h>
    #include <libavutil/samplefmt.h>
    #include <libavutil/timestamp.h>
    #include <libavcodec/avcodec.h>
    #include <libavformat/avformat.h>
    #include <libswscale/swscale.h>
    #define __STDC_CONSTANT_MACROS
}

#include "ffmpeg_decode.hpp"
#include <cassert>
#include <iostream>

 int VideoDecoder_ffmpegImpl::output_video_frame(AVFrame *frame)
 {
     if (frame->width != m_width || frame->height != m_height ||
       frame->format != m_pix_fmt) {
         /* To handle this change, one could call av_image_alloc again and
          * decode the following frames into another rawvideo file. */
         fprintf(stderr, "Error: Width, height and pixel format have to be "
                 "constant in a rawvideo file, but the width, height or "
                 "pixel format of the input video changed:\n"
                 "old: width = %d, height = %d, format = %s\n"
                 "new: width = %d, height = %d, format = %s\n",
                 m_width, m_height, av_get_pix_fmt_name(m_pix_fmt),
                 frame->width, frame->height,
                 av_get_pix_fmt_name((AVPixelFormat)frame->format));
         return -1;
     }
 
     printf("video_frame n:%d\n", m_video_frame_count++);
 
     /* copy decoded frame to destination buffer:
      * this is required since rawvideo expects non aligned data */
     std::cout << "frame->width:" << frame->width   << std::endl;
     std::cout << "frame->height:" << frame->height << std::endl;
     std::cout << "frame->format:" << frame->format << std::endl;

     av_image_copy2(m_video_dst_data, m_video_dst_linesize,
        frame->data, frame->linesize,
                    m_pix_fmt, m_width, m_height);
 
     /* write to rawvideo file */
     fwrite(m_video_dst_data[0], 1, m_video_dst_bufsize, m_video_dst_file);
     return 0;
 }
 
//  int VideoDecoder_ffmpegImpl::output_audio_frame()
//  {
//      size_t unpadded_linesize = m_frame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)frame->format);
//      printf("audio_frame n:%d nb_samples:%d pts:%s\n",
//         m_audio_frame_count++, m_frame->nb_samples,
//         av_ts2timestr_cpp(m_frame->pts, &m_audio_dec_ctx->time_base));
//  
//      fwrite(m_frame->extended_data[0], 1, unpadded_linesize, m_audio_dst_file);
//  
//      return 0;
//  }
 
int VideoDecoder_ffmpegImpl::decode_img(AVCodecContext* dec_ctx, 
    AVFrame* frame, 
    AVPacket* pkt,
    char *frame_type, 
    int32_t **motion_vectors, 
    int32_t *num_mvs 
    )
    {
    //struct SwsContext* sws_ctx = NULL;
    //char buf[1024];
    int ret = -1;
    int sts;

    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0)
    {
        fprintf(stderr, "Error sending a packet for decoding\n");
        return ret;
    }

    //Create SWS Context for converting from decode pixel format (like YUV420) to RGB
    ////////////////////////////////////////////////////////////////////////////
    m_sws_ctx = sws_getCachedContext(
                            m_sws_ctx,
                            dec_ctx->width,
                            dec_ctx->height,
                            dec_ctx->pix_fmt,
                            dec_ctx->width,
                            dec_ctx->height,
                            AV_PIX_FMT_RGB24,
                            SWS_BICUBIC,
                            NULL,
                            NULL,
                            NULL);

    if (m_sws_ctx == nullptr)
    {
        // return;  //Error!
        return ret;
    }
    ////////////////////////////////////////////////////////////////////////////


    //Allocate frame for storing image converted to RGB.
    ////////////////////////////////////////////////////////////////////////////
    //AVFrame* pRGBFrame = av_frame_alloc();

    m_RGBFrame->format = AV_PIX_FMT_RGB24;
    m_RGBFrame->width = dec_ctx->width;
    m_RGBFrame->height = dec_ctx->height;

    sts = av_frame_get_buffer(m_RGBFrame, 0);
    if (sts < 0)
    {
        return sts;
        // return;  //Error!
    }
    ////////////////////////////////////////////////////////////////////////////

    while (ret >= 0) 
    {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret < 0) {
            // those two return values are special and mean there is no output
            // frame available, but there were no errors during decoding
            if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
            {
                //std::cerr <<  "EOF during decoding\n";
                return 0;
            }
            fprintf(stderr, "Error during decoding (%s)\n", av_err2str_cpp(ret));
            return ret;
        }

        printf("saving frame %ld \n", dec_ctx->frame_num);
        fflush(stdout);

        /* the picture is allocated by the decoder. no need to
           free it */
        //snprintf(buf, sizeof(buf), "%s_%03d.pgm", filename, dec_ctx->frame_number);
        //pgm_save(frame->data[0], frame->linesize[0],
        //    frame->width, frame->height, buf);
         
        //Convert from input format (e.g YUV420) to RGB and save to PPM:
        ////////////////////////////////////////////////////////////////////////////
        sts = sws_scale(m_sws_ctx,                //struct SwsContext* c,
                        frame->data,            //const uint8_t* const srcSlice[],
                        frame->linesize,        //const int srcStride[],
                        0,                      //int srcSliceY, 
                        frame->height,          //int srcSliceH,
                        m_RGBFrame->data,        //uint8_t* const dst[], 
                        m_RGBFrame->linesize);   //const int dstStride[]);

        if (sts != frame->height)
        {
            std::cerr <<  "sts != frame->height\n";
            return -1;
            //return;  //Error!
        }
        std::cout << "pRGBFrame->width:" << m_RGBFrame->width << std::endl;
        std::cout << "pRGBFrame->height:" << m_RGBFrame->height << std::endl;
        char frame_type[2] = {'?'};
        std::vector<AVMotionVector> motion_vectors;
        retrieve_motion( frame_type, motion_vectors ); 
        std::cout << "motion_vectors:" << motion_vectors.size() << std::endl;
        std::cout << "frame_type:" << frame_type << std::endl;

            
        
        //snprintf(buf, sizeof(buf), "%s_%03d.ppm", filename, dec_ctx->frame_num);
        //ppm_save(pRGBFrame->data[0], pRGBFrame->linesize[0], pRGBFrame->width, pRGBFrame->height, buf);
        ////////////////////////////////////////////////////////////////////////////
        av_frame_unref(frame);
    }

    //Free
    //av_frame_unref(m_RGBFrame);
    //sws_freeContext(m_sws_ctx);
    //av_frame_free(&m_RGBFrame);
    return 0;
}

 int VideoDecoder_ffmpegImpl::decode_packet(AVCodecContext *dec, 
    const AVPacket *pkt, 
    AVFrame* frame)
 {
     int ret = 0;
 
     // submit the packet to the decoder
     ret = avcodec_send_packet(dec, pkt);
     if (ret < 0) {
         fprintf(stderr, "Error submitting a packet for decoding (%s)\n", av_err2str_cpp(ret));
         return ret;
     }
 
     // get all the available frames from the decoder
     while (ret >= 0) {
         ret = avcodec_receive_frame(dec, frame);
         if (ret < 0) {
             // those two return values are special and mean there is no output
             // frame available, but there were no errors during decoding
             if (ret == AVERROR_EOF || ret == AVERROR(EAGAIN))
                 return 0;
 
             fprintf(stderr, "Error during decoding (%s)\n", av_err2str_cpp(ret));
             return ret;
         }
 
         // write the frame data to output file
         if (dec->codec->type == AVMEDIA_TYPE_VIDEO)
             ret = output_video_frame(frame);
         //else
         //    ret = output_audio_frame(m_frame);
 
         av_frame_unref(frame);
     }
 
     return ret;
 }
 
 int VideoDecoder_ffmpegImpl::open_codec_context(int *stream_idx,
                               AVCodecContext **dec_ctx, enum AVMediaType type)
 {
     int ret, stream_index;
     AVStream *st;
     const AVCodec *dec = NULL;
 
     ret = av_find_best_stream(m_fmt_ctx, type, -1, -1, NULL, 0);
     if (ret < 0) {
         fprintf(stderr, "Could not find %s stream in input file '%s'\n",
                 av_get_media_type_string(type), m_src_filename);
         return ret;
     } else {
         stream_index = ret;
         st = m_fmt_ctx->streams[stream_index];
 
         /* find decoder for the stream */
         dec = avcodec_find_decoder(st->codecpar->codec_id);
         if (!dec) {
             fprintf(stderr, "Failed to find %s codec\n",
                     av_get_media_type_string(type));
             return AVERROR(EINVAL);
         }
 
         /* Allocate a codec context for the decoder */
         *dec_ctx = avcodec_alloc_context3(dec);
         if (!*dec_ctx) {
             fprintf(stderr, "Failed to allocate the %s codec context\n",
                     av_get_media_type_string(type));
             return AVERROR(ENOMEM);
         }
 
         /* Copy codec parameters from input stream to output codec context */
         if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) {
             fprintf(stderr, "Failed to copy %s codec parameters to decoder context\n",
                     av_get_media_type_string(type));
             return ret;
         }
 
         /* Init the decoders */
         if ((ret = avcodec_open2(*dec_ctx, dec, NULL)) < 0) {
             fprintf(stderr, "Failed to open %s codec\n",
                     av_get_media_type_string(type));
             return ret;
         }
         *stream_idx = stream_index;
     }
 
     return 0;
 }
 
int VideoDecoder_ffmpegImpl::get_format_from_sample_fmt(const char **fmt,
                                       enum AVSampleFormat sample_fmt)
 {
     unsigned int i;
     struct sample_fmt_entry {
         enum AVSampleFormat sample_fmt; const char *fmt_be, *fmt_le;
     } sample_fmt_entries[] = {
         { AV_SAMPLE_FMT_U8,  "u8",    "u8"    },
         { AV_SAMPLE_FMT_S16, "s16be", "s16le" },
         { AV_SAMPLE_FMT_S32, "s32be", "s32le" },
         { AV_SAMPLE_FMT_FLT, "f32be", "f32le" },
         { AV_SAMPLE_FMT_DBL, "f64be", "f64le" },
     };
     *fmt = NULL;
 
     for (i = 0; i < FF_ARRAY_ELEMS(sample_fmt_entries); i++) {
         struct sample_fmt_entry *entry = &sample_fmt_entries[i];
         if (sample_fmt == entry->sample_fmt) {
             *fmt = AV_NE(entry->fmt_be, entry->fmt_le);
             std::cout << "fmt:" << *fmt << std::endl;
             return 0;
         }
     }
 
     fprintf(stderr,
             "sample format %s is not supported as output format\n",
             av_get_sample_fmt_name(sample_fmt));
     return -1;
 }

 void VideoDecoder_ffmpegImpl::decode_encode(
    const char* src_filename,
    const char* video_dst_filename
)
 {    
    m_src_filename = src_filename;
    int ret = 0;
    int video_stream_idx = -1; 
    int audio_stream_idx = -1;
    AVCodecContext * video_dec_ctx;
     /* open input file, and allocate format context */
     if (avformat_open_input(&m_fmt_ctx, src_filename, NULL, NULL) < 0) {
        fprintf(stderr, "Could not open source file %s\n", src_filename);
        exit(1);
    }

    /* retrieve stream information */
    if (avformat_find_stream_info(m_fmt_ctx, NULL) < 0) {
        fprintf(stderr, "Could not find stream information\n");
        exit(1);
    }

    if (open_codec_context(&video_stream_idx, &video_dec_ctx, AVMEDIA_TYPE_VIDEO) >= 0) {
        m_video_stream = m_fmt_ctx->streams[video_stream_idx];

        m_video_dst_file = fopen(video_dst_filename, "wb");
        if (!m_video_dst_file) {
            fprintf(stderr, "Could not open destination file %s\n", video_dst_filename);
            ret = 1;
            clean_up_exit();
        }

        /* allocate image where the decoded image will be put */
        m_width   =  video_dec_ctx->width;
        m_height  =  video_dec_ctx->height;
        m_pix_fmt =  video_dec_ctx->pix_fmt;

        std::cout << "m_width:" << m_width << std::endl;
        std::cout << "m_height:" << m_height << std::endl;
        std::cout << "m_pix_fmt:" << m_pix_fmt << std::endl;

        ret = av_image_alloc(m_video_dst_data, m_video_dst_linesize,
            m_width, m_height, m_pix_fmt, 1);
        if (ret < 0) {
            fprintf(stderr, "Could not allocate raw video buffer\n");
            clean_up_exit();
        }
        m_video_dst_bufsize = ret;
    }

    /*   
    if (open_codec_context(&m_audio_stream_idx, &m_audio_dec_ctx, m_fmt_ctx, AVMEDIA_TYPE_AUDIO) >= 0) {
        audio_stream = fmt_ctx->streams[m_audio_stream_idx];
        m_audio_dst_file = fopen(audio_dst_filename, "wb");
        if (!audio_dst_file) {
            fprintf(stderr, "Could not open destination file %s\n", audio_dst_filename);
            ret = 1;
            goto end;
        }
    }*/

    /* dump input information to stderr */
    av_dump_format(m_fmt_ctx, 0, src_filename, 0);

    if (!m_audio_stream && !m_video_stream) {
        fprintf(stderr, "Could not find audio or video stream in the input, aborting\n");
        ret = 1;
        clean_up_exit();
    }

    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate frame\n");
        ret = AVERROR(ENOMEM);
        clean_up_exit();
    }

    //Allocate frame for storing image converted to RGB.
    ////////////////////////////////////////////////////////////////////////////
    m_RGBFrame = av_frame_alloc();
    if (!m_RGBFrame) {
        fprintf(stderr, "Could not allocate RGB frame\n");
        ret = AVERROR(ENOMEM);
        clean_up_exit();
    }

    AVPacket* pkt = av_packet_alloc();
    if (!pkt) {
        fprintf(stderr, "Could not allocate packet\n");
        ret = AVERROR(ENOMEM);
        clean_up_exit();
    }

    if (m_video_stream)
        printf("Demuxing video from file '%s' into '%s'\n", src_filename, video_dst_filename);
    // if (audio_stream)
    //     printf("Demuxing audio from file '%s' into '%s'\n", src_filename, audio_dst_filename);

    /* read frames from the file */
    while (av_read_frame(m_fmt_ctx, pkt) >= 0) {
        // check if the packet belongs to a stream we are interested in, otherwise
        // skip it
        if (pkt->stream_index == video_stream_idx)
        {
            char frame_type[2] = "?"; 
            int32_t *motion_vectors=nullptr; 
            int32_t num_mvs;         
            ret = decode_img(video_dec_ctx, frame,  
                pkt, frame_type, 
                &motion_vectors, &num_mvs);
        }
        else if (pkt->stream_index == audio_stream_idx)
            assert(false && "audio decodeing NOT implemented");
            //ret = decode_packet(audio_dec_ctx, pkt);
        av_packet_unref(pkt);
        if (ret < 0)
            break;
    }

    /* flush the decoders */
    if (video_dec_ctx)
        decode_packet(video_dec_ctx, NULL, frame);
    //if (m_audio_dec_ctx)
    //    decode_packet(m_audio_dec_ctx, NULL);

    printf("Demuxing succeeded.\n");

    if (m_video_stream) {
        printf("Play the output video file with the command:\n"
               "ffplay -f rawvideo -pix_fmt %s -video_size %dx%d %s\n",
               av_get_pix_fmt_name(m_pix_fmt), m_width, m_height,
               video_dst_filename);
    }

    // if (audio_stream) {
    //     enum AVSampleFormat sfmt = audio_dec_ctx->sample_fmt;
    //     int n_channels = audio_dec_ctx->ch_layout.nb_channels;
    //     const char *fmt;
    // 
    //     if (av_sample_fmt_is_planar(sfmt)) {
    //         const char *packed = av_get_sample_fmt_name(sfmt);
    //         printf("Warning: the sample format the decoder produced is planar "
    //                "(%s). This example will output the first channel only.\n",
    //                packed ? packed : "?");
    //         sfmt = av_get_packed_sample_fmt(sfmt);
    //         n_channels = 1;
    //     }
    // 
    //     if ((ret = get_format_from_sample_fmt(&fmt, sfmt)) < 0)
    //         goto end;
    // 
    //     printf("Play the output audio file with the command:\n"
    //            "ffplay -f %s -ac %d -ar %d %s\n",
    //            fmt, n_channels, audio_dec_ctx->sample_rate,
    //            audio_dst_filename);
    // }

 }

 void VideoDecoder_ffmpegImpl::clean_up_exit()
{
    if (m_video_dst_file)
        fclose(m_video_dst_file);
    //if (audio_dst_file)
    //    fclose(m_audio_dst_file);
    av_free(m_video_dst_data[0]);
    av_frame_free(&m_frame);
    av_packet_free(&m_pkt);
    av_frame_unref(m_RGBFrame);
    sws_freeContext(m_sws_ctx);
    av_frame_free(&m_RGBFrame);

    avformat_close_input(&m_fmt_ctx);
    avcodec_free_context(&m_video_dec_ctx);
    avcodec_free_context(&m_audio_dec_ctx);
    exit(1);
}

 int main (int argc, char **argv)
 {
     int ret = 0;
 
     if (argc != 3) {
         fprintf(stderr, "usage: %s  input_file video_output_file audio_output_file\n"
                 "API example program to show how to read frames from an input file.\n"
                 "This program reads frames from a file, decodes them, and writes decoded\n"
                 "video frames to a rawvideo file named video_output_file, and decoded\n"
                 "audio frames to a rawaudio file named audio_output_file.\n",
                 argv[0]);
         exit(1);
     }
    char* src_filename;
    char* video_dst_filename;
    src_filename = argv[1];
    video_dst_filename = argv[2];
    VideoDecoder_ffmpegImpl codec ;
    codec.decode_encode(src_filename, video_dst_filename);
 
     return ret < 0;
 }
 
     // uint8_t **frame, 
    // int *step, 
    // int *width, 
    // int *height, 
    // int *cn, 


 bool VideoDecoder_ffmpegImpl::retrieve_motion(
    char *frame_type,
    std::vector<AVMotionVector> &motion_vectors
    ) 
    {

    if (!m_video_stream || !(this->m_frame->data[0]))
        return false;

    // change color space of frame
    sws_scale(
        m_sws_ctx,
        m_frame->data,
        m_frame->linesize,
        0, m_video_dec_ctx->coded_height,
        m_RGBFrame->data,
        m_RGBFrame->linesize
        );

    //*frame = this->picture.data;
    //*width = this->picture.width;
    //*height = this->picture.height;
    //*step = this->picture.step;
    //*cn = this->picture.cn;

    // get motion vectors
    AVFrameSideData *sd = av_frame_get_side_data(m_frame, AV_FRAME_DATA_MOTION_VECTORS);
    if (sd) {
        AVMotionVector *mvs = (AVMotionVector *)sd->data;

        int num_mvs = sd->size / sizeof(*mvs);


        if (num_mvs > 0) {

            // store the motion vectors 
            for (MVS_DTYPE i = 0; i < num_mvs; ++i) {
                motion_vectors.push_back(mvs[i]);
            }
        }
    }

    // get frame type (I, P, B, etc.) and create a null terminated c-string
    frame_type[0] = av_get_picture_type_char(m_frame->pict_type);
    frame_type[1] = '\0';

    // return the timestamp which was computed previously in grab()
    //*frame_timestamp = this->frame_timestamp;

    return true;
}
