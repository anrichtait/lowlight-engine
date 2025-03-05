/**
  \file G3D-gfx.lib/include/G3D-gfx/VideoStream.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_gfx_VideoStream_h

#include "G3D-base/platform.h"
#include "G3D-base/Array.h"

namespace G3D {

class NetSendConnection;
class NetConnection;
class Texture;

/** \brief Server-side class for streaming low-latency lossy video with GPU MPEG encoding.

    This manages the clients rather than simply performing the encoding so that it can communicate
    with the network asynchronously rather than blocking in send(). All communication for the 
    video streaming is on the VIDEO_NET_CHANNEL, which the application should not use
    for other communication.
    
    \sa G3D::VideoStreamClient
   */
class VideoStreamServer : public ReferenceCountedObject {
protected:
    Array<shared_ptr<NetConnection>>    m_clientArray;

    /** Is this the first frame after a reset? */
    bool                                m_firstFrame                = true;

    VideoStreamServer(const Array<shared_ptr<NetConnection>>& clientArray);

public:
    /** Messages are sent on this channel, allowing them to be scheduled asynchronously from
        other messages in the system when sending. The receiver will accumulate all messages
        into a single receipt queue. */
    static const NetChannel             VIDEO_NET_CHANNEL;
   
    static shared_ptr<VideoStreamServer> create(const Array<shared_ptr<NetConnection>>& clientArray = Array<shared_ptr<NetConnection>>());

    const Array<shared_ptr<NetConnection>>& clientConnectionArray() const {
       return m_clientArray;
    }

    void addClient(const shared_ptr<NetConnection>& client);

    /** Clients that have disconnected are automatically removed during send(). Invoke
        removeClient() to explicitly remove a live connection. */
    void removeClient(const shared_ptr<NetConnection>& client);

    /** Video is initialized on the first frame and must have the same
       resolution after that. Threadsafe. Must be called on the OpenGL thread. */
    void send(const shared_ptr<Texture>& frame);
};


/** \sa G3D::VideoStreamServer */
class VideoStreamClient : public ReferenceCountedObject {
protected:

    shared_ptr<Texture>                     m_texture;
    const shared_ptr<NetConnection>         m_server;
    
    VideoStreamClient(const shared_ptr<NetConnection>& server);

public:
    static const NetChannel             VIDEO_NET_CHANNEL;

    static shared_ptr<VideoStreamClient> create(const shared_ptr<NetConnection>& server);
    static shared_ptr<VideoStreamClient> create(const NetAddress& serverAddress) {
        return create(NetConnection::connectToServer(serverAddress));
    }

    const shared_ptr<NetConnection>& serverConnection() {
        return m_server;
    }

    /** Returns null if there is no next frame available in the queue yet.
        Threadsafe.  Must be called on the OpenGL thread. May block for a short period of time.
        Output format is always SRGB8().  
        
        Calling this may re-use the texture from the previous call for efficiency, so do not
        invoke it until the previous texture is no longer in use by the application.

        Invoke in a while loop until receive() returns nullptr to avoid frames backlogging.

        Will return nullptr if the next message is not on the VIDEO_NET_CHANNEL,
        at which point the application must clear the non-video messages from the
        message iterator.
    */
    shared_ptr<Texture> receive();
};

}
