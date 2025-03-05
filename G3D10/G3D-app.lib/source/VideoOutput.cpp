/**
  \file G3D-app.lib/source/VideoOutput.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/platform.h"
#include "G3D-base/Log.h"
#include "G3D-base/Image.h"
#include "G3D-base/CPUPixelTransferBuffer.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-gfx/GLPixelTransferBuffer.h"
#include "G3D-app/VideoOutput.h"
#include "G3D-app/VideoInput.h"

#ifdef G3D_NO_FFMPEG
    #pragma message("Warning: FFMPEG and VideoOutput are disabled in this build (" __FILE__ ")") 
#else

extern "C" {
    #include "libavfilter/buffersink.h"
    #include "libavfilter/buffersrc.h"
    #include "libavformat/avformat.h"
    #include "libavformat/avio.h"
    #include "libavcodec/avcodec.h"
    #include "libavutil/avutil.h"
    #include "libavutil/opt.h"
    #include "libswscale/swscale.h"
}


namespace G3D {

// helper used by VideoInput and VideoOutput to capture error logs from ffmpeg
void initFFmpegLogger();

VideoOutput::Encoder VideoOutput::Encoder::DEFAULT() {
    Encoder e;
    e.codecId = -1;
    e.description = "Default MPEG-4/H.264 (.mp4)";
    e.extension = ".mp4";
    return e;
}

VideoOutput::Encoder VideoOutput::Encoder::H265_NVIDIA() {
    Encoder e;
    e.codecName = "hevc_nvenc";
    e.description = "H.265/HEVC NVIDIA (.mp4)";
    e.extension = ".mp4";

    // default preset is already 'medium'
    // default profile is already 'main'
    return e;
}

VideoOutput::Encoder VideoOutput::Encoder::H264_NVIDIA() {
    Encoder e;
    e.codecName = "h264_nvenc";
    e.description = "H.264/AVC NVIDIA (.mp4)";
    e.extension = ".mp4";

    // default preset is already 'medium'
    // default profile is already 'main'
    return e;
}

VideoOutput::Encoder VideoOutput::Encoder::MPEG4() {
    Encoder e;
    e.codecName = "mpeg4";
    e.description = "MPEG-4 Generic (.mp4)";
    e.extension = ".mp4";

    // set max b-frames to 2 to help with compression
    e.options.set("bf", "2");
    return e;
}

void VideoOutput::Settings::setBitrateQuality(float quality) {
    debugAssertM(width > 0 && height > 0, "Must set width and height before quality level.");
    bitrate = (int)(1500 * 1024 * (float)(width * height) / (640 * 480));
}

VideoOutput::Settings::Settings()
    : width(0), height(0), fps(0), bitrate(0), flipVertical(false), encoder(Encoder::DEFAULT()) {}

shared_ptr<VideoOutput> VideoOutput::create(const String& filename, const Settings& settings) {
    shared_ptr<VideoOutput> vo = createShared<VideoOutput>(filename, settings);

    Array<Encoder> encoders;
    if (settings.encoder.codecId == -1) {
        // default encoder - list all mpeg-4/h.264 encoders
        encoders.append(Encoder::H264_NVIDIA());
        encoders.append(Encoder::MPEG4());
    } else {
        // use specified encoder only
        encoders.append(settings.encoder);
    }

    // loop through all encoders until one works
    for (int i = 0; i < encoders.length(); ++i) {
        vo->m_settings.encoder = encoders[i];
        if (vo->initialize()) {
            return vo;
        }
        vo->abort();
    }

    return nullptr;
}


VideoOutput::VideoOutput(const String& filename, const Settings& settings)
    : m_filename(filename)
    , m_settings(settings)
    , m_isFinished(false)
    , m_framecount(0)
    , m_avFormatContext(nullptr)
    , m_avVideoContext(nullptr)
    , m_avVideoStream(nullptr)
    , m_avOptions(nullptr)
    , m_avBufferSrc(nullptr)
    , m_avBufferSink(nullptr)
    , m_avFilterGraph(nullptr)
{
}


VideoOutput::~VideoOutput() {
    shutdown();
}

bool VideoOutput::initialize() {
    initFFmpegLogger();
    m_isFinished = false;

    // initialize list of available muxers/demuxers and codecs in ffmpeg
    av_register_all();
    avfilter_register_all();

    // validate settings
    if (!validSettings()) {
        debugPrintf("VideoOutput: Invalid settings\n");
        return false;
    }


    // try to create AVFormatContext from filename
    int r = avformat_alloc_output_context2(&m_avFormatContext, nullptr, nullptr, m_filename.c_str());
    if (!m_avFormatContext) {
        // Print available formats
        {
            debugPrintf("Available VideoOutput formats are:\n");
            AVOutputFormat* oformat = av_oformat_next(nullptr);
            while (oformat) {
                debugPrintf("  %s\n", oformat->long_name);
                oformat = av_oformat_next(oformat);
            }
        }
        
        debugPrintf("VideoOutput: bad format context (%d)\n", r);
        return false;
    }

    #ifdef G3D_OSX
    debugPrintf("VideoOutput: avformat_alloc_output_context2 returned %d\n", r);
    #endif

    // add video stream
    AVCodec* codec = nullptr;
    if (!m_settings.encoder.codecName.empty()) {
        // find specific codec by name if specified
        codec = avcodec_find_encoder_by_name(m_settings.encoder.codecName.c_str());
    } else {
        // otherwise add first codec of codec id type
        codec = avcodec_find_encoder(static_cast<AVCodecID>(m_settings.encoder.codecId));
    }

    if (!codec) {
        debugPrintf("VideoOutput: bad codec\n");
        return false;
    }

    m_avVideoStream = avformat_new_stream(m_avFormatContext, codec);
    if (!m_avVideoStream) {
        debugPrintf("VideoOutput: no video stream\n");
        return false;
    }

    m_avVideoStream->id = m_avFormatContext->nb_streams - 1;

    m_avVideoContext = avcodec_alloc_context3(codec);
    if (!m_avVideoContext) {
        debugPrintf("VideoOutput: no video context\n");
        return false;
    }

    // setup basic codec settings
    m_avVideoContext->width = m_settings.width;
    m_avVideoContext->height = m_settings.height;
    m_avVideoContext->bit_rate = m_settings.bitrate;
    m_avVideoContext->framerate = { m_settings.fps, 1 };
    m_avVideoContext->time_base = { 1, m_settings.fps };

    // set initial stream time_base, ffmpeg will scale it later
    // any timestamps from the codec must be scaled to match the stream time_base
    m_avVideoStream->time_base = { 1, m_settings.fps };

    // set pixel format -- default to YUV420P if available
    const AVPixelFormat* pixelFormat = codec->pix_fmts;
    while (*pixelFormat != AV_PIX_FMT_NONE) {
        if (*pixelFormat == AV_PIX_FMT_YUV420P) {
            m_avVideoContext->pix_fmt = AV_PIX_FMT_YUV420P;
            break;
        }
        ++pixelFormat;
    }

    if (*pixelFormat == AV_PIX_FMT_NONE) {
        m_avVideoContext->pix_fmt = codec->pix_fmts[0];
    }

    // some formats want stream headers to be separate.
    if (m_avFormatContext->oformat->flags & AVFMT_GLOBALHEADER) {
        m_avVideoContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    // make sure refcounted frames are enabled anywhere they default to off
    av_dict_set(&m_avOptions, "refcounted_frames", "1", 0);

    // add codec-specific options
    for (auto iter = m_settings.encoder.options.begin(); iter != m_settings.encoder.options.end(); ++iter) {
        av_dict_set(&m_avOptions, iter->key.c_str(), iter->value.c_str(), 0);
    }

    // open the codec with options
    int ret = avcodec_open2(m_avVideoContext, codec, &m_avOptions);
    if (ret < 0) {
        debugPrintf("VideoOutput: could not open codec\n");
        return false;
    }

    ret = avcodec_parameters_from_context(m_avVideoStream->codecpar, m_avVideoContext);
    if (ret < 0) {
        debugPrintf("VideoOutput: could not get parameters\n");
        return false;
    }

    // open output file for writing
    ret = avio_open(&m_avFormatContext->pb, m_filename.c_str(), AVIO_FLAG_WRITE);
    if (ret < 0) {
        debugPrintf("VideoOutput: could not open output file\n");
        return false;
    }

    // start the stream
    ret = avformat_write_header(m_avFormatContext, &m_avOptions);
    if (ret < 0) {
        debugPrintf("VideoOutput: could not write header\n");
        return false;
    }

    // create filter graph
    m_avFilterGraph = avfilter_graph_alloc();
    if (m_avFilterGraph == nullptr) {
        debugPrintf("VideoOutput: could not create filter graph\n");
        return false;
    }

    // create source (buffer) and output(buffersink)
    const AVFilter *buffersrc = avfilter_get_by_name("buffer");
    const AVFilter *buffersink = avfilter_get_by_name("buffersink");

    // create source filter
    String args = format("video_size=%dx%d:pix_fmt=%d:time_base=1/%d", m_settings.width, m_settings.height, AV_PIX_FMT_RGB24, m_settings.fps);
    ret = avfilter_graph_create_filter(&m_avBufferSrc, buffersrc, "in", args.c_str(), nullptr, m_avFilterGraph);
    if (ret < 0) {
        debugPrintf("VideoOutput: could not create in filter\n");
        return false;
    }

    // create output filterm_avBufferSrc
    ret = avfilter_graph_create_filter(&m_avBufferSink, buffersink, "out", nullptr, nullptr, m_avFilterGraph);
    if (ret < 0) {
        debugPrintf("VideoOutput: could not create out filter\n");
        return false;
    }

    // set output format
    AVPixelFormat fmts[] = { m_avVideoContext->pix_fmt, AV_PIX_FMT_NONE };
    ret = av_opt_set_int_list(m_avBufferSink, "pix_fmts", fmts, AV_PIX_FMT_NONE, AV_OPT_SEARCH_CHILDREN);
    if (ret < 0) {
        debugPrintf("VideoOutput: could not set options\n");
        return false;
    }

    AVFilterInOut *outputs = avfilter_inout_alloc();
    AVFilterInOut *inputs = avfilter_inout_alloc();

    outputs->name = av_strdup("in");
    outputs->filter_ctx = m_avBufferSrc;
    outputs->pad_idx = 0;
    outputs->next = nullptr;

    inputs->name = av_strdup("out");
    inputs->filter_ctx = m_avBufferSink;
    inputs->pad_idx = 0;
    inputs->next = nullptr;

    String filters;
    if (m_settings.flipVertical) {
        filters = "vflip";
    }
    
    ret = avfilter_graph_parse_ptr(m_avFilterGraph, "null", &inputs, &outputs, nullptr);
    if (ret < 0) {
        avfilter_inout_free(&inputs);
        avfilter_inout_free(&outputs);
        debugPrintf("VideoOutput: could parse graph\n");
        return false;
    }

    ret = avfilter_graph_config(m_avFilterGraph, nullptr);
    avfilter_inout_free(&inputs);
    avfilter_inout_free(&outputs);

    if (ret < 0) {
        debugPrintf("VideoOutput: could configure graph\n");
        return false;
    }
    return true;
}
    

void VideoOutput::shutdown() {
    // check if destroying before commit()
    abort();

    if (m_avVideoContext) {
        avcodec_free_context(&m_avVideoContext);
    }

    if (m_avFormatContext) {
        avformat_free_context(m_avFormatContext);
        m_avFormatContext = nullptr;
    }

    av_dict_free(&m_avOptions);
}

bool VideoOutput::validSettings() {
    if (m_settings.width < 2 || m_settings.height < 2 || m_settings.fps < 1.0f) {
        return false;
    }
    return true;
}

void VideoOutput::append(RenderDevice* rd, bool backbuffer) {
    debugAssert(rd->width() == m_settings.width);
    debugAssert(rd->height() == m_settings.height);

    RenderDevice::ReadBuffer old = rd->readBuffer();
    if (backbuffer) {
        rd->setReadBuffer(RenderDevice::READ_BACK);
    } else {
        rd->setReadBuffer(RenderDevice::READ_FRONT);
    }
    debugAssertGLOk();


    // TODO: Optimize using GLPixelTransferBuffer and glReadPixels instead of screenshotPic
    shared_ptr<Image> image = rd->screenshotPic(false, true);
    rd->setReadBuffer(old);
    shared_ptr<CPUPixelTransferBuffer> imageBuffer = dynamic_pointer_cast<CPUPixelTransferBuffer>(image->toPixelTransferBuffer());
    encodeFrame(static_cast<const uint8*>(imageBuffer->buffer()), imageBuffer->format());
}


void VideoOutput::append(const shared_ptr<Texture>& frame, bool invertY) {
    debugAssert(frame->width() == m_settings.width);
    debugAssert(frame->height() == m_settings.height);

    const shared_ptr<PixelTransferBuffer>& buffer = frame->toPixelTransferBuffer(TextureFormat::RGB8());
    encodeFrame(static_cast<const uint8*>(buffer->mapRead()), ImageFormat::RGB8());
    buffer->unmap();
}


void VideoOutput::append(const shared_ptr<PixelTransferBuffer>& frame) {
    debugAssert(frame->width() == m_settings.width);
    debugAssert(frame->height() == m_settings.height);

    encodeFrame(static_cast<const uint8*>(frame->mapRead()), frame->format());
    frame->unmap();
}



void VideoOutput::encodeFrame(const uint8* pixels, const ImageFormat* format) {
    if (m_isFinished) {
        return;
    }

    AVFrame* frame = av_frame_alloc();
    frame->format = AV_PIX_FMT_RGB24;
    frame->width = m_settings.width;
    frame->height = m_settings.height;

    int ret = av_frame_get_buffer(frame, 0);
    if (ret >= 0) {
        av_frame_make_writable(frame);

        // copy each line individually to accomodate padding in the AVFrame buffer for alignment
        const int sourceLineSize = frame->width * ImageFormat::RGB8()->cpuBitsPerPixel / 8;
        runConcurrently(0, frame->height, [&](int y) {
            memcpy(frame->data[0] + (y * frame->linesize[0]), pixels + (y * sourceLineSize), sourceLineSize);
        });

        frame->pts = ++m_framecount;

        ret = av_buffersrc_add_frame_flags(m_avBufferSrc, frame, AV_BUFFERSRC_FLAG_KEEP_REF);
        if (ret >= 0) {
            AVFrame* filteredFrame = av_frame_alloc();
            while (1) {
                ret = av_buffersink_get_frame(m_avBufferSink, filteredFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                    break;
                }

                if (ret >= 0) {
                    ret = avcodec_send_frame(m_avVideoContext, filteredFrame);
                    av_frame_unref(filteredFrame);

                    if (ret >= 0) {
                        AVPacket* packet = av_packet_alloc();
                        while (ret >= 0) {
                            ret = avcodec_receive_packet(m_avVideoContext, packet);
                            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
                                break;
                            }

                            if (ret >= 0) {
                                av_packet_rescale_ts(packet, m_avVideoContext->time_base, m_avVideoStream->time_base);
                                packet->stream_index = m_avVideoStream->index;

                                ret = av_interleaved_write_frame(m_avFormatContext, packet);
                                av_packet_unref(packet);
                                debugAssert(ret >= 0);
                            }
                        }

                        av_packet_free(&packet);
                    }
                }
            }
            av_frame_free(&filteredFrame);
        }
    }

    av_frame_free(&frame);
}


void VideoOutput::commit() {
    m_isFinished = true;

    AVFrame* filteredFrame = nullptr;

    // flush the filter graph first to make sure no new frames there
    int ret = av_buffersrc_add_frame_flags(m_avBufferSrc, nullptr, AV_BUFFERSRC_FLAG_KEEP_REF);
    if (ret >= 0) {
        filteredFrame = av_frame_alloc();

        ret = av_buffersink_get_frame(m_avBufferSink, filteredFrame);
        if (ret < 0) {
            // no new frame, so set to nullptr to flush the encoder
            av_frame_free(&filteredFrame);
        }
    }

    ret = avcodec_send_frame(m_avVideoContext, filteredFrame);
    av_frame_free(&filteredFrame);

    if (ret >= 0) {
        AVPacket* packet = av_packet_alloc();
        while (ret >= 0) {
            ret = avcodec_receive_packet(m_avVideoContext, packet);

            if (ret >= 0) {
                av_packet_rescale_ts(packet, m_avVideoContext->time_base, m_avVideoStream->time_base);
                packet->stream_index = m_avVideoStream->index;

                ret = av_interleaved_write_frame(m_avFormatContext, packet);
                av_packet_unref(packet);
                debugAssert(ret >= 0);
            }
        }

        av_packet_free(&packet);
    }

    // write the trailer to create a valid file
    av_write_trailer(m_avFormatContext);

    avio_closep(&m_avFormatContext->pb);
}

void VideoOutput::abort() {
    m_isFinished = true;
    if (m_avFormatContext && m_avFormatContext->pb) {
        avio_closep(&m_avFormatContext->pb);

#       ifdef _MSC_VER
            _unlink(m_filename.c_str());
#       else
            unlink(m_filename.c_str());
#       endif //_MSC_VER
    }
}

static void ffmpegLogger(void*, int level, const char* fmt, va_list args) {
    Log::common()->print("[ffmpeg] ");
    Log::common()->vprintf(fmt, args);
}

// helper used by VideoInput and VideoOutput to capture error logs from ffmpeg
void initFFmpegLogger() {
    static bool initialized = false;
    if (!initialized) {
        initialized = true;
        av_log_set_level(AV_LOG_ERROR);
#ifdef G3D_OSX // TODO: remove
        av_log_set_level(AV_LOG_TRACE);
        #endif
        av_log_set_callback(ffmpegLogger);
    }
}

} // namespace G3D

#endif

