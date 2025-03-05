/**
  \file G3D-app.lib/source/VideoInput.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include "G3D-base/platform.h"
#include "G3D-base/fileutils.h"
#include "G3D-app/VideoInput.h"
#include "G3D-gfx/Texture.h"
#include "G3D-base/CPUPixelTransferBuffer.h"
#include <thread>

#ifdef G3D_NO_FFMPEG
    #pragma message("Warning: FFMPEG and VideoOutput are disabled in this build (" __FILE__ ")") 
#else

extern "C" {
    #include "libavformat/avformat.h"
    #include "libavcodec/avcodec.h"
    #include "libavutil/avutil.h"
    #include "libswscale/swscale.h"
    #include <errno.h>
}


namespace G3D {

// helper used by VideoInput and VideoOutput to capture error logs from ffmpeg
void initFFmpegLogger();

shared_ptr<VideoInput> VideoInput::fromFile(const String& filename) {
    shared_ptr<VideoInput> vi(new VideoInput);

    try {
        vi->initialize(filename);
    } catch (const String& s) {
        // TODO: Throw the exception
        debugAssertM(false, s);(void)s;
        vi.reset();
    }
    return vi;
}


VideoInput::VideoInput() : 
    m_quitThread(false),
    m_avFormatContext(nullptr),
    m_avCodecContext(nullptr),
    m_avStream(nullptr),
    m_avResizeContext(nullptr) {

}

VideoInput::~VideoInput() {
    // shutdown decoding thread
    m_quitThread = true;
    m_thread.wait();

    if (m_avCodecContext) {
        avcodec_close(m_avCodecContext);
        avcodec_free_context(&m_avCodecContext);
    }

    if (m_avFormatContext) {
        avformat_close_input(&m_avFormatContext);
    }

    if (m_avResizeContext) {
        sws_freeContext(m_avResizeContext);
        m_avResizeContext = nullptr;
    }
}

bool VideoInput::initialize(const String& filename) {
    initFFmpegLogger();

    // initialize list of available muxers/demuxers and codecs in ffmpeg
    avcodec_register_all();
    av_register_all();

    if (avformat_open_input(&m_avFormatContext, filename.c_str(), nullptr, nullptr) < 0) {
        return false;
    }

    avformat_find_stream_info(m_avFormatContext, nullptr);
    for (int streamIdx = 0; streamIdx < (int)m_avFormatContext->nb_streams; ++streamIdx) {
        if (m_avFormatContext->streams[streamIdx]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
            m_avStream = m_avFormatContext->streams[streamIdx];
            break;
        }
    }

    if (! m_avStream) {
        return false;
    }

    m_avCodecContext = avcodec_alloc_context3(nullptr);
    if (! m_avCodecContext) {
        return false;
    }

    if (avcodec_copy_context(m_avCodecContext, m_avStream->codec) < 0) {
        return false;
    }

    // Initialize the codecc
    AVCodec* codec = avcodec_find_decoder(m_avCodecContext->codec_id);
    if (avcodec_open2(m_avCodecContext, codec, nullptr) < 0) {
        return false;
    }

    // Create resize context since the parameters shouldn't change throughout the video
    m_avResizeContext = sws_getContext(m_avCodecContext->width, m_avCodecContext->height, m_avCodecContext->pix_fmt, 
                                       m_avCodecContext->width, m_avCodecContext->height, AV_PIX_FMT_RGB24, SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (! m_avResizeContext) {
        return false;
    }
    
    // everything is setup and ready to be decoded
    m_thread = std::async(std::launch::async, VideoInput::decode, this);
    return true;
}

int VideoInput::width() const {
    return m_avCodecContext->width;
}

int VideoInput::height() const {
    return m_avCodecContext->height;
}

float VideoInput::fps() const {
    return (float)av_q2d(m_avCodecContext->framerate);
}

RealTime VideoInput::length() const {
    return m_avStream->duration * av_q2d(m_avStream->time_base);
}

bool VideoInput::finished() const {
    const bool threadFinished = (m_thread.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
    return threadFinished && (m_frames.size() == 0);
}

const ImageFormat* VideoInput::imageFormat() {
    return ImageFormat::SRGB8();
}

shared_ptr<CPUPixelTransferBuffer> VideoInput::nextFrame() {
    shared_ptr<CPUPixelTransferBuffer> buffer;
    m_frames.popFront(buffer);
    return buffer;
}

bool VideoInput::nextFrame(shared_ptr<Texture> frame) {
    shared_ptr<CPUPixelTransferBuffer> buffer;
    if (m_frames.popFront(buffer)) {
        if (frame->format() == ImageFormat::SRGB8() || frame->format() == ImageFormat::RGB8()) {
            if (frame->width() == width() && frame->height() == height()) {
                // update existing texture
                glBindTexture(frame->openGLTextureTarget(), frame->openGLID());
                glPixelStorei(GL_PACK_ALIGNMENT, 1);

                const void* readBuffer = buffer->mapRead();
                glTexImage2D(frame->openGLTextureTarget(), 0, frame->format()->openGLFormat, frame->width(), frame->height(), 0,
                    TextureFormat::RGB8()->openGLBaseFormat, TextureFormat::RGB8()->openGLDataFormat, readBuffer);
                buffer->unmap();

                glBindTexture(frame->openGLTextureTarget(), GL_NONE);
                return true;
            }
        }
    }
    return false;
}

bool VideoInput::nextFrame(shared_ptr<PixelTransferBuffer> frame) {
    shared_ptr<CPUPixelTransferBuffer> buffer;
    if (m_frames.popFront(buffer)) {
        if (frame->format() == ImageFormat::SRGB8() || frame->format() == ImageFormat::RGB8()) {
            if (frame->width() == width() && frame->height() == height()) {
                // copy frame
                memcpy(frame->mapWrite(), buffer->mapRead(), buffer->size());
                frame->unmap();
                buffer->unmap();
                return true;
            }
        }
    }
    return false;
}

bool VideoInput::decode(VideoInput* vi) {
    // todo: adjust this based on stream fps
    const int maxFrames = 5;

    while (! vi->m_quitThread) {

        // check of frame queue is full
        if (vi->m_frames.size() >= maxFrames) {
            // todo: adjust this based on stream fps
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        AVPacket packet;
        av_init_packet(&packet);

        // decode next frame
        if (av_read_frame(vi->m_avFormatContext, &packet) < 0) {
            // return on error or end of file
            av_free_packet(&packet);
            break;
        }

        // ignore frames other than our video frame
        if (packet.stream_index == vi->m_avStream->index) {
            AVFrame* decodingFrame = av_frame_alloc();

            // decode the frame
            int completedFrame = 0;
            avcodec_decode_video2(vi->m_avCodecContext, decodingFrame, &completedFrame, &packet);

            // we have a valid frame, let's use it!
            if (completedFrame != 0) {
                shared_ptr<CPUPixelTransferBuffer> buffer = CPUPixelTransferBuffer::create(vi->m_avCodecContext->width, vi->m_avCodecContext->height, ImageFormat::RGB8());

                uint8_t* destPlanes[] = { (uint8_t*)buffer->mapWrite() };
                int destStrides[] = { (int)buffer->stride() };

                // Convert the image from its native format to RGB
                sws_scale(vi->m_avResizeContext, decodingFrame->data, decodingFrame->linesize, 0, vi->m_avCodecContext->height, destPlanes, destStrides);
                
                buffer->unmap();

                vi->m_frames.pushBack(buffer);
            }

            av_frame_free(&decodingFrame);
        }   

        av_free_packet(&packet);
    }
    return true;
}


shared_ptr<VideoPlayer> VideoPlayer::fromFile(const String& filename) {
    shared_ptr<VideoPlayer> player(new VideoPlayer(filename));
    if (isNull(player->m_video)) {
        return nullptr;
    }

    player->m_texture = Texture::createEmpty("video", player->width(), player->height(), VideoInput::imageFormat());
    return player;
}

shared_ptr<VideoPlayer> VideoPlayer::fromFile(const String& filename, shared_ptr<Texture> frameTexture) {
    shared_ptr<VideoPlayer> player(new VideoPlayer(filename));
    if (isNull(player->m_video)) {
        return nullptr;
    }
    player->m_texture = frameTexture;
    return player;
}

shared_ptr<VideoPlayer> VideoPlayer::fromFile(const String& filename, shared_ptr<PixelTransferBuffer> frameBuffer) {
    shared_ptr<VideoPlayer> player(new VideoPlayer(filename));
    if (isNull(player->m_video)) {
        return nullptr;
    }
    player->m_buffer = frameBuffer;
    return player;
}

VideoPlayer::VideoPlayer(const String& filename)
    : m_time(0.0)
    , m_paused(false) {

    m_video = VideoInput::fromFile(filename);
}

bool VideoPlayer::update(RealTime timestep) {
    if (m_paused) {
        return false;
    }

    m_time -= timestep;
    if (m_time <= 0) {
        bool updated = false;
        if (notNull(m_texture)) {
            updated = m_video->nextFrame(m_texture);
        }
        if (notNull(m_buffer)) {
            updated = m_video->nextFrame(m_buffer);
        }

        if (updated) {
            m_time = 1.0 / fps();
        } else {
            m_time = 0.0;
        }
    }
    return false;
}


} // namespace G3D

#endif
