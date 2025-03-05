/**
  \file G3D-base.lib/include/G3D-base/WebServer.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#pragma once
#define G3D_base_WebServer_h

#include "G3D-base/platform.h"
#include "G3D-base/ReferenceCount.h"
#include "G3D-base/ThreadsafeQueue.h"
#include "G3D-base/Array.h"
#include "G3D-base/NetAddress.h"
#include <time.h>
#include <mutex>

struct mg_context;
struct mg_callbacks;
struct mg_connection;
struct mg_request_info;

namespace G3D {

/**
 \brief Web server with support for https and websockets
 */
class WebServer : public ReferenceCountedObject {
public:
    static const uint16 DEFAULT_PORT = 8080;

    class Specification {
    public:
        uint16                      port;

        /** Defaults to the www subdirectory of the current directory */
        String                      fileSystemRoot;

        Specification(uint16 port = DEFAULT_PORT);

    };
    
    /** Subclass this if your websockets do not need to share state across all instances of a URI handler per-WebServer instance.
        If you do need to share data, subclass */
    class WebSocket : public ReferenceCountedObject {
    public:
        enum Opcode {
            /*	        CONTINUATION = 0x0,*/
	        TEXT = 0x1,
	        BINARY = 0x2,
            /*
	        CONNECTION_CLOSE = 0x8,
	        PING = 0x9,
	        PONG = 0xa */
        };

        const NetAddress                clientAddress;

    protected:
        WebServer*                      m_server;
        struct mg_connection*           m_connection;
        
        WebSocket(WebServer* server, mg_connection* connection, const NetAddress& clientAddress) :
            clientAddress(clientAddress), m_server(server), m_connection(connection) {}

    public:

        /** Returns the number of bytes written, 0 if the connection was closed, and -1 on error */
        virtual int send(Opcode opcode, const uint8* data, size_t dataLen);

        /** Sends as TEXT */
        int send(const String& s) {
            return send(TEXT, reinterpret_cast<const uint8*>(s.c_str()), s.size() + 1);
        }

        /** Sends as BINARY */
        int send(const class BinaryOutput& b);

        /** Return true to accept this connection.
            Default implementation returns true.
            Invoked on arbitrary threads.
            */
        virtual bool onConnect() { return true; };

        /** Invoked when both sides have accepted the connection.
            Invoked on arbitrary threads. */
        virtual void onReady() { };

        /** Invoked when data appears. Return true to maintain the connection.

            Note that the Opcode may be values other than TEXT and BINARY, in which
            case programs should ignore the message and return true unless they
            have special ways of interacting at a low level with the WebSocket protocol.

            Default returns true; Invoked on arbitrary threads. */
        virtual bool onData(Opcode opcode, char* data, size_t data_len) {
            return true;
        }

        /** Invoked on arbitrary threads. */
        virtual void onClose() {}
    };
   
    typedef std::function<shared_ptr<WebSocket>(WebServer*, mg_connection*, const NetAddress&)> SocketFactory;

protected:

    static int websocket_connect_handler(const struct mg_connection* conn, void* user_data);
    static void websocket_ready_handler(struct mg_connection* conn, void* user_data);
    static int websocket_data_handler(struct mg_connection* conn, int flags, char* data, size_t data_len, void* user_data);
    static void websocket_close_handler(const struct mg_connection* conn, void* userdata);

    class SocketScheme {
    public:
        WebServer*                      webServer;
        String                          uri;
        SocketFactory                   factory;
        Array<shared_ptr<WebSocket>>    webSocketArray;

        SocketScheme(WebServer* webServer, const String& uri, SocketFactory factory) : webServer(webServer), uri(uri), factory(factory) {}
    };
    
    /** Maps URI schemes to SocketSchemes. Protected by m_socketTableMutex */
    Table<String, SocketScheme*>        m_socketSchemeTable;

    /** Protected by m_socketTableMutex */
    Table<mg_connection*, shared_ptr<WebSocket>> m_socketTable;
    mutable std::mutex                  m_socketTableMutex;

    Specification                       m_specification;
    mg_context*                         m_context;

    WebServer(const Specification&);

    virtual ~WebServer();
    
    shared_ptr<WebSocket> socketFromConnection(mg_connection* connection) const;

    /** Default implementation invokes socket->onReady */
    virtual void onWebSocketReady(const shared_ptr<WebSocket>& socket) { socket->onReady(); }
    virtual bool onWebSocketConnect(const shared_ptr<WebSocket>& socket) { return socket->onConnect(); };
    virtual bool onWebSocketData(const shared_ptr<WebSocket>& socket, WebSocket::Opcode opcode, char* data, size_t data_len) {
        return socket->onData(opcode, data, data_len);
    }

    virtual void onWebSocketClose(const shared_ptr<WebSocket>& socket) { socket->onClose(); }

public:

    /** The server is not started on creation. Invoke start(). */
    static shared_ptr<WebServer> create(const Specification& specification = Specification());
    
    /** Register a set of event handlers for a specific websocket URI (e.g., "/websocket", "/lobby", "/game").

        Example:

        \begincode
        class MySocket : public WebServer::WebSocket {
        protected:
            MySocket(WebServer* server, const NetAddress& clientAddress) : WebSocket(server, clientAddress) {}
        public:
            static shared_ptr<WebSocket> create(WebServer* server, const NetAddress& clientAddress) {
                return createShared<MySocket>(server, clientAddress);
            }

            bool onData(int flags, char* data, size_t data_len) override {
                printf("Received data: %s\n", data);
                return true;
            }
        };

        server->registerWebSocketHandler("/print", &MySocket::create);
        \endcode

        \sa getWebSocketArray
    */
    void registerWebSocketHandler(const String& uri, SocketFactory socketFactory);

    /** Get all web sockets that are currently open and responding to this uri. Useful 
        for iterating through clients. Threadsafe. 
        
        \param uri The uri parameter passed to registerWebSocketHandler to facilitate the creation of these sockets.
        */
    void getWebSocketArray(const String& uri, Array<shared_ptr<WebSocket>>& array);

    void start();

    /** The destructor automatically invokes stop(). */
    void stop();
};

}


