/**
  \file G3D-app.lib/include/G3D-app/VideoInput.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#pragma once
#include "G3D-base/platform.h"
#include "G3D-base/G3DString.h"
#include "G3D-base/Rect2D.h"
#include "G3D-base/ReferenceCount.h"
#include "G3D-base/ThreadsafeQueue.h"
#include <future>

#ifndef G3D_NO_FFMPEG

// forward declarations for ffmpeg
struct AVFormatContext;
struct AVCodecContext;
struct AVStream;
struct SwsContext;

namespace G3D {

class CPUPixelTransferBuffer;
class ImageFormat;
class PixelTransferBuffer;
class Texture;

/**
    Read video files from MPG, MP4, AVI, MOV, OGG, ASF, and WMV files.

    Simply returns the next available frames until the video is finished.
    Requires a properly formatted SRGB8 or RGB8 Texture or PixelTransferBuffer
    otherwise will not copy the frame.  Use imageFormat() to create the supported
    format.

    Use VideoPlayer to playback a video at the correct speed.

    \sa VideoPlayer
*/
class VideoInput : public ReferenceCountedObject {
public:
    /** @return nullptr if unable to open file or video is not supported  */
    static shared_ptr<VideoInput> fromFile(const String& filename);
    ~VideoInput();

    int width() const;
    int height() const;
    float fps() const;
    RealTime length() const;
    bool finished() const;

    /** @return Recommended ImageFormat for Texture or PixelTransferBuffer */
    static const ImageFormat* imageFormat();

    /** @return The buffer containing the next available frame or nullptr if no available frame */
    shared_ptr<CPUPixelTransferBuffer> nextFrame();

    /** Copies the next available frame into the Texture *
        @return true if frame was available and copied, false otherwise */
    bool nextFrame(shared_ptr<Texture> frame);

    /** Copies the next available frame into the Texture *
        @return true if frame was available and copied, false otherwise */
    bool nextFrame(shared_ptr<PixelTransferBuffer> frame);

private:
    VideoInput();

    bool initialize(const String& filename);

    static bool decode(VideoInput* vi);
    std::future<bool>  m_thread;
    std::atomic_bool   m_quitThread;

    ThreadsafeQueue<shared_ptr<CPUPixelTransferBuffer>> m_frames;

    // ffmpeg management
    AVFormatContext*    m_avFormatContext;
    AVCodecContext*     m_avCodecContext;
    AVStream*           m_avStream;
    SwsContext*         m_avResizeContext;
};

/**
    Play videos back at the correct speed and automatically
    update frame Textur or PixelTransferBuffer.

    \sa VideoInput
*/
class VideoPlayer : public ReferenceCountedObject {
public:
    /** Creates a Texture that each frame is automatically copied into.
        The Texture can be accessed by calling frameTexture().
        @return nullptr if unable to open file or video is not supported  */
    static shared_ptr<VideoPlayer> fromFile(const String& filename);
    /** Sets the Texture that each frame is automatically copied into.
        The Texture can be accessed by calling frameTexture().
        @return nullptr if unable to open file or video is not supported  */
    static shared_ptr<VideoPlayer> fromFile(const String& filename, shared_ptr<Texture> frameTexture);
    /** Sets the PixelTransferBuffer that each frame is automatically copied into.
        The buffer can be accessed by calling frameBuffer().
        @return nullptr if unable to open file or video is not supported  */
    static shared_ptr<VideoPlayer> fromFile(const String& filename, shared_ptr<PixelTransferBuffer> frameBuffer);

    int width() const { return m_video->width();  }
    int height() const { return m_video->height(); }
    float fps() const { return m_video->fps(); }
    RealTime length() const { return m_video->length(); }
    bool finished() const { return m_video->finished();  }

    shared_ptr<Texture> frameTexture() const { return m_texture; }
    shared_ptr<PixelTransferBuffer> frameBuffer() const { return m_buffer; }

    /** Updates the video playback time and updates the frame texture or buffer if needed.
        @return true if the frame was updated, false otherwise */
    bool update(RealTime timestep);
    
    void pause() { m_paused = true; }
    void unpause() { m_paused = false; }
    bool paused() const { return m_paused; }

private:
    VideoPlayer(const String& filename);

    shared_ptr<VideoInput>  m_video;
    RealTime                m_time;
    bool                    m_paused;

    shared_ptr<Texture>             m_texture;
    shared_ptr<PixelTransferBuffer> m_buffer;
};

} // namespace G3D

#endif