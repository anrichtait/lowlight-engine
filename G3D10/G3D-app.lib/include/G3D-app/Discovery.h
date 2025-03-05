/**
  \file G3D-app.lib/include/G3D-app/Discovery.h

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#pragma once
#ifndef G3D_app_Discovery_h
#define G3D_app_Discovery_h

#include <cstring>
#include <climits>
#include "G3D-base/NetAddress.h"
#include "G3D-base/network.h"
#include "G3D-gfx/OSWindow.h"
#include "G3D-app/Widget.h"
#include "G3D-app/GuiWindow.h"
#include "G3D-app/GuiTheme.h"

namespace G3D {

class BinaryInput;
class BinaryOutput;
class GuiPane;
class GuiLabel;
class GuiButton;

namespace Discovery {

    /**
    Process:
    1. Server ---- ServerDescription ---> ServiceDiscovery
    2. Client ---- ClientQuery ---> ServiceDiscovery
    3. ServiceDiscovery ---- vector<ServerDescription> ---> Client
    4. Client picks a server
    5. Client ---- [Application Stuff] --->  Server
    6. Server ---- updated ServerDescription ---> ServiceDiscovery
    7. ServiceDiscovery ---- updated vector<ServerDescription> ---> Client

    Send vector -> want to completely update the list
    ServiceDiscovery needs to keep track of servers vs clients
    */
    enum {
        CLIENT_QUERY_TYPE = 44144,
        SERVER_DESCRIPTION_TYPE = 10101
    };

    static const NetChannel CHANNEL = 0;
    static const uint16 DEFAULT_SERVICE_DISCOVERY_PORT = 6175;
    static const uint16 DEFAULT_SERVER_PORT = 6176;

    /** \brief Used by G3D::Discovery::Client to request 
    * specific servers from ServiceDiscovery
    */
    class ClientQuery {
    public:
        String                   applicationName;

        inline ClientQuery() : applicationName(System::appName()) {}

        ClientQuery(BinaryInput& b);

        void serialize(BinaryOutput& b) const;

        void deserialize(BinaryInput& b);
    };

    /** \brief Used by G3D::Discovery::Server to advertise its services.


    */
  
    class ServerDescription {
    public:

        /** Address on which the server is listening for incoming
            application (not discovery) connections. */
        NetAddress               applicationAddress;

        /** Name of the application.  Clients only display servers
            for applications that have the same name as themselves.

            Include a version number in this if you wish to distinguish
            between application versions.
        */
        String                   applicationName;

        /** Name of the server for display.  This need not have any
            relationship to the hostname of the server.*/
        String                   serverName;

        /** Maximum number of clients the server is willing to accept. */
        int                      maxClients;

        /** Number of clients currently connected to the server.*/
        int                      currentClients;

        /** Application specific data.  This is not displayed by the
         built-in server browser.  It is for storing application specific
         data like the name of the map for a game.

         See G3D::TextInput for parsing if the data is complicated.*/
        String                   data;

        /** On the client side, the last time this server was checked.  Unused on the server side.*/
        RealTime                 lastUpdateTime;

        inline ServerDescription(const NetAddress& address = NetAddress(NetAddress::localHostname(), Discovery::DEFAULT_SERVER_PORT),
            const String& applicationName = System::appName(), 
            const String& serverName = System::appName() + "@" + NetAddress::localHostname(),
            const int& maxClients = INT_MAX, 
            const int& currentClients = 0 
        ) : applicationAddress(address), applicationName(applicationName), serverName(serverName), maxClients(maxClients), currentClients(currentClients), lastUpdateTime(System::time()) {}

        inline ServerDescription(const uint16& port,
            const String& applicationName = System::appName(), 
            const String& serverName = System::appName() + "@" + NetAddress::localHostname(),
            const int& maxClients = INT_MAX, 
            const int& currentClients = 0
        ) : applicationAddress(NetAddress(NetAddress::localHostname(), port)), applicationName(applicationName), serverName(serverName), maxClients(maxClients), currentClients(currentClients), lastUpdateTime(System::time()) {}

        ServerDescription(BinaryInput& b);

        String displayText() const;

        void serialize(BinaryOutput& b) const;

        void deserialize(BinaryInput& b);
    };


    /**
      Options for configuring the G3D Discovery protocol.
      These rarely need to be changed except for the client-side
      display options.

      The default constructor looks for semi-colon separated service 
      discovery addresses in the G3D10_DISCOVERY_SETTINGS environment 
      variable.
    @beta
     */
    class Settings {

    public:
        String applicationName;

        /** Address on which service discovery receives requests*/
        Array<NetAddress>       serviceDiscoveryAddresses;



        inline Settings(
            const String& appName,
            const Array<NetAddress>& addresses) :
            applicationName(appName),
            serviceDiscoveryAddresses(addresses) {}

        Settings();
    };

    /**
       To use the built-in browser UI, call Client::create.

       To write your own UI, call Client::createNoGui.

       Invoke onNetwork() periodically (e.g., at 30 fps or higher) to
       manage network requests from Service Discovery.  This can be done 
       automatically by calling G3D::GApp::addWidget() or 
       G3D::WidgetManager::add() with the client as an argument at the 
       start of a program.

       See samples/netchat for an example of use
    @beta
     */
    class Client : public GuiWindow {

        static const int windowWidth = 500; 
        static const int windowHeight = 300; 

        Settings                  m_settings;

        /** Connection to Service Discovery */
        shared_ptr<NetConnection>  m_serviceDiscoveryConnection;

        Array<ServerDescription>  m_serverDescriptions;

        ClientQuery               m_clientQuery;

        NetAddress                m_selectedServerAddress;
        int                       m_selectedServerIndex;

        GuiPane* m_viewPane;
        Array<GuiRadioButton*> m_serverButtons; 

        Client(const ClientQuery& clientQuery, const Settings& settings);

        Client(const ClientQuery& clientQuery, const Settings& settings,
            const shared_ptr<GuiTheme>& theme);


        /** Called from onNetwork() to receive incomming messages from service Discovery */
        void receiveDescriptions();

        virtual void initNetwork();

    public:

        /** \brief Creates an invisible discovery client that maintains a
            list of available servers.

            This is suitable for compute servers.

            \sa create
         */
        static shared_ptr<Client> createNoGui(const ClientQuery& clientQuery = ClientQuery(), const Settings& settings = Settings());

        /** \brief Creates a Gui-based server browser.
        This is suitable for games.
        \sa createNoGui */
        static shared_ptr<Client> create(const ClientQuery& clientQuery = ClientQuery(),
            const shared_ptr<GuiTheme>& theme = shared_ptr<GuiTheme>(),
            const Settings& settings = Settings());

        virtual void onNetwork();

        virtual bool onEvent(const GEvent& e); 

        /** @brief All servers that have been discovered */
        inline const Array<ServerDescription>& serverDescriptions() const {
            return m_serverDescriptions;
        }

        inline const Settings& settings() const {
            return m_settings;
        }

        /** @brief The server address selected in the GuiWindow */
        inline const NetAddress& selectedServerAddress() const {
            return m_selectedServerAddress;
        }

        // Sends a request to Server Discovery
        virtual void queryDiscovery();
    };

    /**
       @brief Advertises a service on this machine for other clients.

       Invoke onNetwork() periodically (e.g., at 30 fps or higher) to
       manage network requests.  This can be done automatically by calling
       G3D::GApp::addWidget() or G3D::WidgetManager::add() with the server
       as an argument at the start of a program.

       @beta
    */
    class Server : public Widget {
    private:

        Settings                   m_settings;

        /** Connection to Service Discovery */
        shared_ptr<NetConnection>  m_serviceDiscoveryConnection;

        /**
            Description of the properties of this server.
            Update at any time.
         */
        ServerDescription          m_description;

        /** Last time the server advertised. */
        RealTime                   m_lastAdvertisementTime;

        /** Broadcast the m_description on all adapters */
        void sendDescriptionToDiscovery();

        Server(const ServerDescription& description, const Settings& settings);

    public:

        static shared_ptr<Server> create(const ServerDescription& description,
            const Settings& settings = Settings());

        inline const Settings& settings() const {
            return m_settings;
        }

        inline const ServerDescription& description() {
            return m_description;
        }

        /** Triggers immediate advertising of this description.
         */
        void setDescription(const ServerDescription& d);

        virtual void onNetwork();
    };

    /**
       To use the built-in browser UI, call ServiceDiscovery::create.

       To write your own UI, call ServiceDiscovery::createNoGui.
       
       Invoke onNetwork() periodically (e.g., at 30 fps or higher) to
       manage network requests from Service Discovery.  This can be done 
       automatically by calling G3D::GApp::addWidget() or 
       G3D::WidgetManager::add() with the ServiceDiscovery pointer as an 
       argument at the start of a program.

       See samples/matchmaker for an example of use
    @beta
     */
    class ServiceDiscovery : public GuiWindow {
    private:

        static const int windowWidth = 500; 
        static const int windowHeight = 300;

        struct ServerData {
            ServerDescription serverDesc;
            shared_ptr<NetConnection> serverConnection;
            GuiLabel* guiLabel; 
        };

        struct ClientData {
            ClientQuery clientQuery;
            shared_ptr<NetConnection> clientConnection;
        };

        Settings                m_settings;

        shared_ptr<NetServer>   m_net;

        Array<shared_ptr<NetConnection>> m_newConnections;

        Array<ServerData>  m_serverConnections;

        Array<ClientData>  m_clientConnections;

        /** viewPane contains all server labels  */
        GuiPane* m_viewPane;

        inline ServiceDiscovery(const shared_ptr<NetServer>& net, const Settings& settings) :m_settings(settings), m_net(net), m_newConnections(), m_viewPane(nullptr) {}

        ServiceDiscovery(const shared_ptr<NetServer>& net, const Settings& settings, const shared_ptr<GuiTheme>& theme);

        // Send all Matching Server Descriptions to the specified Client
        virtual void sendDescriptionsToClient(const ClientData& client) const;

        // Send all Matching Server Descriptions to all Clients
        virtual void sendDescriptionsAll(String& appName) const;

        /** 
        Called from onNetwork() to process new connections. 
        Returns true if connection has been sorted as either client or server
        */
        virtual bool processNewConnection(const shared_ptr<NetConnection>& connection);

        // Processes Server Description received from Server
        virtual void receiveDescription(ServerData& server, NetMessageIterator& msg);

        virtual void updateClientQuery(ClientData& client, NetMessageIterator& msg);

    public:

        /** \brief Creates an invisible discovery client that maintains a
            list of available servers.

            This is suitable for compute servers.

            \sa create
         */
        static shared_ptr<ServiceDiscovery> createNoGui(const shared_ptr<NetServer>& net, const Settings& settings = Settings());

        /** \brief Creates a Gui-display of availabler servers
        \sa createNoGui */
        static shared_ptr<ServiceDiscovery> create(const shared_ptr<NetServer>& net,
            const shared_ptr<GuiTheme>& theme = shared_ptr<GuiTheme>(),
            const Settings& settings = Settings());


        virtual void onNetwork() override;

        /** @brief All servers that have been discovered */
        inline const void serverDescriptions(Array<ServerDescription>& serverDescs) const {
            for (const ServerData& server : m_serverConnections) {
                serverDescs.append(server.serverDesc);
            }
        }

        inline const Settings& settings() const {
            return m_settings;
        }
    };

} // Discovery
} // G3D
#endif // Discovery_h