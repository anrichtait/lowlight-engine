/**
  \file G3D-base.lib/source/WebServer.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/
#include "G3D-base/WebServer.h"
#include "G3D-base/FileSystem.h"
#include "G3D-base/BinaryOutput.h"
#include <functional>
#include <civetweb.h>

namespace G3D {

WebServer::Specification::Specification(uint16 port) : port(port) {
    fileSystemRoot = FilePath::concat(FileSystem::currentDirectory(), "www");
}

/////////////////////////////////////////////////////////////////////

WebServer::WebServer(const Specification& specification) : m_specification(specification), m_context(nullptr) {
}


WebServer::~WebServer() {    
    Array<shared_ptr<WebSocket>> all;
    m_socketTableMutex.lock();
    m_socketTable.getValues(all);
    m_socketTableMutex.unlock();

    // Notify all sockets that they're being closed
    for (const shared_ptr<WebSocket>& socket : all) {
        this->onWebSocketClose(socket);
    }

    stop();
    m_socketSchemeTable.deleteValues();
}


shared_ptr<WebServer> WebServer::create(const Specification& specification) {
	return createShared<WebServer>(specification);
}


static void printRequest(const mg_request_info* request_info) {
    const NetAddress client(request_info->remote_addr, request_info->remote_port);

    debugPrintf("Client at %s performed %s on URI \"%s\", which contains data \"%s\"\n",
                client.ipString().c_str(), request_info->request_method, request_info->request_uri,
                request_info->query_string ? request_info->query_string : "(nullptr)");
    for (int i = 0; i < request_info->num_headers; ++i) {
        debugPrintf("   %s: %s\n", request_info->http_headers[i].name, request_info->http_headers[i].value);
    }
}

/** Returns 1 to accept the connection, 0 to reject. */
int WebServer::websocket_connect_handler(const struct mg_connection* connection, void* user_data) {
    debugAssert(user_data);
    WebServer::SocketScheme* socketScheme = reinterpret_cast<WebServer::SocketScheme*>(user_data);
    WebServer* webServer = socketScheme->webServer;

    const struct mg_request_info* ri = mg_get_request_info(connection);

    const shared_ptr<WebSocket>& webSocket =
        socketScheme->factory(webServer,
            const_cast<mg_connection*>(connection),
            NetAddress(ri->remote_addr, ri->remote_port));

    // add socket to two sets
    webServer->m_socketTableMutex.lock();
    socketScheme->webSocketArray.append(webSocket);
    webServer->m_socketTable.set(const_cast<mg_connection*>(connection), webSocket);
    webServer->m_socketTableMutex.unlock();

    // Note that the return value is reversed relative to the data handler, following civetweb's API:
    // true -> 0, false -> 1
    return webServer->onWebSocketConnect(webServer->socketFromConnection(const_cast<mg_connection*>(connection))) ? 0 : 1;
}


shared_ptr<WebServer::WebSocket> WebServer::socketFromConnection(mg_connection* connection) const {
    m_socketTableMutex.lock();
    // Can't be a const reference because we're about to release the mutex
    shared_ptr<WebSocket> webSocket = m_socketTable[connection];
    m_socketTableMutex.unlock();

    return webSocket;
}


void WebServer::websocket_ready_handler(struct mg_connection* connection, void* user_data) {
    debugAssert(user_data);
    WebServer::SocketScheme* socketScheme = reinterpret_cast<WebServer::SocketScheme*>(user_data);
    WebServer* webServer = socketScheme->webServer;
    webServer->onWebSocketReady(webServer->socketFromConnection(connection));
}


int WebServer::websocket_data_handler(struct mg_connection* connection, int opcode, char* data, size_t data_len, void* user_data) {
    debugAssert(user_data);
    WebServer::SocketScheme* socketScheme = reinterpret_cast<WebServer::SocketScheme*>(user_data);
    WebServer* webServer = socketScheme->webServer;
    return webServer->onWebSocketData(webServer->socketFromConnection(connection), (WebSocket::Opcode)(opcode & 0xF), data, data_len) ? 1 : 0;
}


void WebServer::websocket_close_handler(const struct mg_connection* connection, void* userdata) {
    debugAssert(userdata);
    WebServer::SocketScheme* socketScheme = reinterpret_cast<WebServer::SocketScheme*>(userdata);
    WebServer* webServer = socketScheme->webServer;
    webServer->onWebSocketClose(webServer->socketFromConnection(const_cast<mg_connection*>(connection)));

    // Drop the connection from the table
    webServer->m_socketTableMutex.lock();
    webServer->m_socketTable.remove(const_cast<mg_connection*>(connection));
    webServer->m_socketTableMutex.unlock();
}


static int error_handler(struct mg_connection* connection, int status, const char* msg) {
    if (status == 304) {
        // 304 is a request for a value that has not changed since last sent to the client.
        // This is a normal condition for a correctly operating program.

        // debugPrintf("Connection 0x%x: HTTP error %d\n", (unsigned int)(uintptr_t)conn, status);

        const String& content = "Not modified";
        mg_printf(connection, 
                  "HTTP/1.1 304 NOT MODIFIED\r\n"
                  "Content-Type: text/html\r\n"
                  "Content-Length: %d\r\n"
                  "\r\n"
                  "%s", (int)content.size(), content.c_str());
    } else {
        debugPrintf("Connection 0x%x: HTTP error %d: %s\n", (unsigned int)(uintptr_t)connection, status, msg);
        const struct mg_request_info* ri = mg_get_request_info(connection);
        printRequest(ri);

        if (status == 500) {
            // The client already closed the connection
            return 0;
        }

        const String& content =
            format("<html><head><title>Illegal URL</title></head><body>Illegal URL (error %d)</body></html>\n", status);

        mg_printf(connection, 
                  "HTTP/1.1 200 OK\r\n"
                  "Content-Type: text/html\r\n"
                  "Content-Length: %d\r\n"
                  "\r\n"
                  "%s", (int)content.size(), content.c_str());

    }

    // Returning non-zero tells civetweb that our function has replied to
    // the client, and civetweb should not send client any more data.
    return 1;
}


void WebServer::getWebSocketArray(const String& uri, Array<shared_ptr<WebSocket>>& array) {
    SocketScheme* socketScheme = nullptr;

    array.fastClear();
    m_socketTableMutex.lock();

    SocketScheme** socketSchemePtr = m_socketSchemeTable.getPointer(uri);
    if (notNull(socketSchemePtr)) {
        socketScheme = *socketSchemePtr;
    }

    m_socketTableMutex.unlock();

    if (notNull(socketScheme)) {
        array = socketScheme->webSocketArray;
    }
}


void WebServer::start() {
	alwaysAssertM(isNull(m_context), "WebServer was already started");

	// List of options. Last element must be nullptr.
	static const String port = format("%d", m_specification.port);
	static const String root = m_specification.fileSystemRoot;
	const char* options[] =
	{ "listening_ports", port.c_str(),
	  "document_root",   root.c_str(),
	  nullptr };

	// Prepare callbacks structure. We have only one callback, the rest are nullptr.
	mg_callbacks callbacks;
	memset(&callbacks, 0, sizeof(callbacks));
	callbacks.http_error = error_handler;

	// Start the web server.
	m_context = mg_start(&callbacks, nullptr, options);
	debugAssert(notNull(m_context));

}


void WebServer::registerWebSocketHandler(const String& uri, SocketFactory socketFactory) {
    debugAssertM(! m_socketSchemeTable.containsKey(uri), "That URI is already registered for websockets");
    SocketScheme* socketScheme = new SocketScheme(this, uri, socketFactory);
    m_socketSchemeTable.set(uri, socketScheme);
    mg_set_websocket_handler(m_context, uri.c_str(),
                         websocket_connect_handler,
                         websocket_ready_handler,
                         websocket_data_handler,
                         websocket_close_handler,
                         socketScheme);
}


void WebServer::stop() {
	if (notNull(m_context)) {
		mg_stop(m_context);
		m_context = nullptr;
	}
}


int WebServer::WebSocket::send(Opcode opcode, const uint8* data, size_t dataLen) {
    return mg_websocket_write(m_connection, (int)opcode, reinterpret_cast<const char*>(data), dataLen);
}

int WebServer::WebSocket::send(const BinaryOutput& bo) {
    return send(BINARY, bo.getCArray(), bo.size());
}

}
