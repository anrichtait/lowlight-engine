/**
  \file G3D-app.lib/include/G3D-app/VideoOutput.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#pragma once
#include "G3D-base/G3DString.h"
#include "G3D-base/g3dmath.h"
#include "G3D-base/Image.h"
#include "G3D-base/ReferenceCount.h"
#include "G3D-base/Table.h"
#include "G3D-gfx/Texture.h"

#ifndef G3D_NO_FFMPEG

// forward declarations for ffmpeg
struct AVCodecContext;
struct AVDictionary;
struct AVFilterContext;
struct AVFilterGraph;
struct AVFormatContext;
struct AVStream;

namespace G3D {

/** 
    \brief Creates video files such as mp4/h264 from provided frames or textures. 
 */
class VideoOutput : public ReferenceCountedObject {
public:
    class Encoder {
    public:
        int codecId;
        String codecName;
        String description;
        String extension;

        Table<String, String> options;

        static Encoder DEFAULT();

        static Encoder H265_NVIDIA();
        static Encoder H264_NVIDIA();
        static Encoder MPEG4();

        Encoder() : codecId(0) {}
    };

    class Settings {
    public:
        int width;
        int height;
        int fps;
        int bitrate;
        bool flipVertical;

        Encoder encoder;

        void setBitrateQuality(float quality = 1.0f);

        Settings();
    };

protected:
    VideoOutput(const String& filename, const Settings& settings);

    bool initialize();
    void shutdown();

    bool validSettings();
    void encodeFrame(const uint8* pixels, const ImageFormat* format);

    String              m_filename;
    Settings            m_settings;

    bool                m_isInitialized;
    bool                m_isFinished;
    int                 m_framecount;

    // ffmpeg management
    AVFormatContext*    m_avFormatContext;
    AVCodecContext*     m_avVideoContext;
    AVStream*           m_avVideoStream;
    AVDictionary*       m_avOptions;

    AVFilterContext*    m_avBufferSrc;
    AVFilterContext*    m_avBufferSink;
    AVFilterGraph*      m_avFilterGraph;

public:
    /**
       Video files have a file format and a codec.  VideoOutput
       chooses the file format based on the filename's extension
       (e.g., .avi creates an AVI file) and the codec based on Settings::codec
     */
    static shared_ptr<VideoOutput> create(const String& filename, const Settings& settings);

    ~VideoOutput();

    const String& filename() const { return m_filename; }

    const Settings& settings() const { return m_settings; }

    void append(const shared_ptr<Texture>& frame, bool invertY = false); 

    void append(const shared_ptr<PixelTransferBuffer>& frame); 

    /** @brief Append the current frame on the RenderDevice to this
        video.  


        @param useBackBuffer If true, read from the back
        buffer (the current frame) instead of the front buffer.
        to be replaced/removed.
     */
    void append(class RenderDevice* rd, bool useBackBuffer = false); 

    /** Aborts writing video file and ends encoding */
    void abort();

    /** Finishes writing video file and ends encoding */
    void commit();

    bool finished()       { return m_isFinished; }

};

} // namespace G3D

#endif
