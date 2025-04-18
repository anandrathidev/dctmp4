
#ifndef FFMPEG_DEODE_HPP
#define FFMPEG_DEODE_HPP

#define MVS_DTYPE int32_t

#include <vector>

class VideoDecoder_ffmpegImpl
{
    AVFormatContext *  m_fmt_ctx = NULL;
    AVStream *         m_video_stream = NULL;
    AVStream *         m_audio_stream = NULL;

    AVCodecContext *   m_video_dec_ctx = NULL;
    AVCodecContext *   m_audio_dec_ctx;
    int                m_width;
    int                m_height;
    enum AVPixelFormat m_pix_fmt;
    const char *       m_src_filename = NULL;
//    const char *       m_video_dst_filename = NULL;
//    const char *       m_audio_dst_filename = NULL;
    FILE *             m_video_dst_file = NULL;
    FILE *             m_audio_dst_file = NULL;
    uint8_t*           m_video_dst_data[4] = {NULL};
    int                m_video_dst_linesize[4];
    int                m_video_dst_bufsize;    
    AVFrame*           m_frame = NULL;
    AVFrame*           m_RGBFrame = NULL;
    AVPacket*          m_pkt = NULL;
    int                m_video_frame_count = 0;
    int                m_audio_frame_count = 0;
    char               m_ts_str[AV_TS_MAX_STRING_SIZE] = {0};
    char               m_err_str[AV_TS_MAX_STRING_SIZE] = {0};
    SwsContext*        m_sws_ctx = NULL;    

    
    char* av_ts2timestr_cpp(int64_t ts, AVRational *tb) 
    {
        //  #define av_ts2timestr_cpp(ts, tb) av_ts_make_time_string(ts_str, ts, tb)
        if (ts == AV_NOPTS_VALUE)
            snprintf(m_ts_str, AV_TS_MAX_STRING_SIZE, "NOPTS");
        else
            snprintf(m_ts_str, AV_TS_MAX_STRING_SIZE, "%" PRId64, ts);
        return av_ts_make_time_string(m_ts_str, ts, tb);
    }

    char* av_err2str_cpp(int errnum)
    {
        // #define av_err2str_cpp(errnum) 
        // av_make_error_string(err_str, AV_ERROR_MAX_STRING_SIZE, errnum)
        return av_make_error_string(m_err_str, AV_ERROR_MAX_STRING_SIZE, errnum);
    }

    int output_video_frame(AVFrame *frame);
    // int output_audio_frame();
    int decode_packet(AVCodecContext* dec, const AVPacket* pkt, AVFrame* frame);
    int open_codec_context(int *stream_idx,
        AVCodecContext **dec_ctx, enum AVMediaType type);

    int get_format_from_sample_fmt(const char **fmt, enum AVSampleFormat sample_fmt);

    int decode_img(AVCodecContext* dec_ctx, 
        AVFrame* frame, 
        AVPacket* pkt,
        char *frame_type, 
        int32_t **motion_vectors, 
        int32_t *num_mvs 
        ); 

    public:
    void decode_encode(
        const char* src_filename, 
        const char* video_dst_filename        
    );
    void clean_up_exit();

    bool retrieve_motion(
        char *frame_type, 
        std::vector<AVMotionVector> &motion_vectors
    ); 
        

};

#endif // FFMPEG_DEODE_HPP