/**
  \file G3D-gfx.lib/source/VideoStream.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/platform.h"
#include "G3D-base/network.h"
#include "G3D-gfx/VideoStream.h"
#include "G3D-gfx/Texture.h"

namespace G3D {
const NetChannel VideoStreamServer::VIDEO_NET_CHANNEL           = 0xFFFFFF9C;
const NetChannel VideoStreamClient::VIDEO_NET_CHANNEL           = 0xFFFFFF9C;

/** Packet contains dimensions and encoding. Restart encoding from here. */
static const NetMessageType         RESET_MESSAGE               = 1;

/** Packet contains incremental frame data. */
static const NetMessageType         FRAME_MESSAGE               = RESET_MESSAGE + 1;

VideoStreamServer::VideoStreamServer(const Array<shared_ptr<NetConnection>>& clientArray) : 
    m_clientArray(clientArray) {
}


shared_ptr<VideoStreamServer> VideoStreamServer::create(const Array<shared_ptr<NetConnection>>& clientArray) {
    return createShared<VideoStreamServer>(clientArray);
}


void VideoStreamServer::addClient(const shared_ptr<NetConnection>& client) {
    m_clientArray.append(client);
}


void VideoStreamServer::removeClient(const shared_ptr<NetConnection>& client) {
    const int i = m_clientArray.findIndex(client);
    if (i >= 0) {
        m_clientArray.fastRemove(i);
    }
}


void VideoStreamServer::send(const shared_ptr<Texture>& frame) {
    if (m_clientArray.size() == 0) { return; }

    // The current implementation uses PNG format. Future versions will use H.264 streaming encoding.
    // Individual packets are sent with no metadata so that we can directly memory-map a CUDA buffer.
    // Otherwise we'd have to copy to the CPU into another buffer to add metadata and that process
    // would add more latency.

    BinaryOutput bo("<memory>", G3D_BIG_ENDIAN);
    frame->toImage(ImageFormat::RGB8())->serialize(bo, Image::PNG);

    for (int c = 0; c < m_clientArray.size(); ++c) {
        const shared_ptr<NetConnection>& client = m_clientArray[c];
        switch (client->status()) {
        case NetConnection::NetworkStatus::CONNECTED:
        case NetConnection::NetworkStatus::JUST_CONNECTED:
            client->send(FRAME_MESSAGE, bo, VIDEO_NET_CHANNEL);
            break;

        case NetConnection::NetworkStatus::WAITING_TO_DISCONNECT:
        case NetConnection::NetworkStatus::DISCONNECTED:
            // Remove the dead client to prevent the entire system
            // from crashing when one client leaves.
            m_clientArray.fastRemove(c);
            --c;
            break;

        default:
            // Do nothing while waiting to connect
            ;
        }
    }

}

////////////////////////////////////////////////////////////////////
    
VideoStreamClient::VideoStreamClient(const shared_ptr<NetConnection>& server) : m_server(server) {}


shared_ptr<VideoStreamClient> VideoStreamClient::create(const shared_ptr<NetConnection>& server) {
    return createShared<VideoStreamClient>(server);
}


shared_ptr<Texture> VideoStreamClient::receive() {
    if (m_server->status() != NetConnection::NetworkStatus::CONNECTED) {
        return nullptr;
    }

    NetMessageIterator &iterator = m_server->incomingMessageIterator(VIDEO_NET_CHANNEL);

    shared_ptr<Texture> texture;
        
    while (true)
    {
        if (! iterator.isValid() || (iterator.channel() != VIDEO_NET_CHANNEL))
        {
            // No new messages for this client
            return nullptr;
        }

        switch (iterator.type())
        {
            case RESET_MESSAGE:
                // Nothing to do in the PNG protocol, move on
                // to the next message
                ++iterator;
                break;

            case FRAME_MESSAGE:
            {
                BinaryInput& bi = iterator.headerBinaryInput();
                const shared_ptr<Image>& image = Image::fromBinaryInput(bi, ImageFormat::SRGB8());
                texture = Texture::fromImage("frame", image, ImageFormat::SRGB8(), Texture::DIM_2D, false);
                break;
            }

            default: // Unknown message!
                ++iterator;

        }
    } // while more messages
    

    return texture;
}

}
