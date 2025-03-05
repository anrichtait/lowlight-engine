/**
  \file G3D-app.lib/source/Discovery.cpp

  G3D Innovation Engine http://casual-effects.com/g3d
  Copyright 2000-2019, Morgan McGuire
  All rights reserved
  Available under the BSD License
*/

#include <climits>
#include "G3D-base/platform.h"
#include "G3D-base/BinaryInput.h"
#include "G3D-base/BinaryOutput.h"
#include "G3D-base/Log.h"
#include "G3D-app/Draw.h"
#include "G3D-app/Discovery.h"
#include "G3D-gfx/RenderDevice.h"
#include "G3D-app/GuiLabel.h"
#include "G3D-app/GuiPane.h"


namespace G3D {
namespace Discovery {

ClientQuery::ClientQuery(BinaryInput& b) {
	deserialize(b);
}

void ClientQuery::serialize(BinaryOutput& b) const {
	b.writeString(applicationName);
}


void ClientQuery::deserialize(BinaryInput& b) {
	applicationName = b.readString();
}

/////////////////////////////////////////////////////////////

ServerDescription::ServerDescription(BinaryInput& b) {
	deserialize(b);
}

void ServerDescription::serialize(BinaryOutput& b) const {
	b.writeString(serverName);
	applicationAddress.serialize(b);
	b.writeString(applicationName);
	b.writeInt32(maxClients);
	b.writeInt32(currentClients);
	b.writeString(data);
}


void ServerDescription::deserialize(BinaryInput& b) {
	serverName = b.readString();
	applicationAddress.deserialize(b);
	applicationName = b.readString();
	maxClients = b.readInt32();
	debugAssert(maxClients >= 0);
	currentClients = b.readInt32();
	data = b.readString();
	lastUpdateTime = System::time();
}


String ServerDescription::displayText() const {
	if (maxClients == INT_MAX) {
		// Infinite number of clients
		return
			format("%24s (%6d) %s",
				serverName.c_str(),
				currentClients,
				applicationAddress.toString().c_str());
	}
	else {
		// Finite number of clients
		return
			format("Server:%24s (%3d/%-3d) %s",
				serverName.c_str(),
				currentClients,
				maxClients,
				applicationAddress.toString().c_str());
	}
}

/////////////////////////////////////////////////////////////

Settings::Settings() {
	// look for G3D10_SERVICE_DISCOVERY, if not, hardcoded
	const char* serviceDiscovery = G3D::System::getEnv("G3D10_SERVICE_DISCOVERY");


	if (serviceDiscovery) {
		Array<String> ips = G3D::stringSplit(serviceDiscovery, ';');
		for (auto ip : ips) {
			serviceDiscoveryAddresses.push_back(NetAddress(ip));
		}
	}
	else {
		serviceDiscoveryAddresses = Array<NetAddress>(NetAddress(NetAddress::DEFAULT_ADAPTER_HOST, Discovery::DEFAULT_SERVICE_DISCOVERY_PORT));
	}
	applicationName = System::appName();
}
/////////////////////////////////////////////////////////////

Client::Client(const ClientQuery& clientQuery, const Settings& settings) : m_settings(settings),
	m_clientQuery(clientQuery),
	m_viewPane(nullptr),
	m_selectedServerAddress(NetAddress()),
	m_selectedServerIndex(0) {
	initNetwork();
}

Client::Client(const ClientQuery& clientQuery, const Settings& settings,
	const shared_ptr<GuiTheme>& theme) :
	GuiWindow("Server Browser", theme,
		Rect2D::xywh(100, 100, windowWidth, windowHeight), GuiTheme::NORMAL_WINDOW_STYLE, NO_CLOSE),
	m_settings(settings),
	m_clientQuery(clientQuery),
	m_selectedServerAddress(NetAddress()),
	m_selectedServerIndex(0) {

	GuiScrollPane* scrollPane = pane()->addScrollPane(true, false, GuiTheme::BORDERLESS_SCROLL_PANE_STYLE);
	scrollPane->moveBy(-6, 0);
	scrollPane->setSize(windowWidth - 10, windowHeight - 10);
	m_viewPane = scrollPane->viewPane();
	m_viewPane->setWidth(windowWidth - 20);

	initNetwork();
}

shared_ptr<Client> Client::createNoGui(const ClientQuery& clientQuery, const Settings& settings) {
	return std::shared_ptr<Client>(new Client(clientQuery, settings));
}

shared_ptr<Client> Client::create(const ClientQuery& clientQuery,
	const shared_ptr<GuiTheme>& theme, const Settings& settings) {
	return std::shared_ptr<Client>(new Client(clientQuery, settings, isNull(theme) ? GuiTheme::lastThemeLoaded.lock() : theme));
}

void Client::initNetwork() {
	for (NetAddress addr : m_settings.serviceDiscoveryAddresses) {
		try {
			m_serviceDiscoveryConnection = NetConnection::connectToServer(addr);
			break;
		}
		catch (...) {
			logPrintf("Discovery::Client: Service Discovery at %s not found", addr.toString().c_str());
		}
	}

	queryDiscovery();
}

void Client::onNetwork() {
	if (isNull(m_serviceDiscoveryConnection)) {
		return;
	}

	// Check for service discovery responses
	switch (m_serviceDiscoveryConnection->status()) {
	case NetConnection::JUST_CONNECTED:
		// We've just connected to a new client or server but never invoked send() or incomingMessageIterator()
		receiveDescriptions();
		break;

	case NetConnection::CONNECTED:
		receiveDescriptions();
		break;

	case NetConnection::DISCONNECTED:
		m_serviceDiscoveryConnection = nullptr;
		logPrintf("Discovery::Client: Service Discovery disconnected");
		break;

	default:
		break;
	} // Switch status

	// communications with server
}

void Client::receiveDescriptions() {
	for (NetMessageIterator& msg = m_serviceDiscoveryConnection->incomingMessageIterator(Discovery::CHANNEL); msg.isValid(); ++msg) {
		BinaryInput& bi = msg.binaryInput();

		if (msg.type() == SERVER_DESCRIPTION_TYPE) {

			m_serverDescriptions.clear();
			if (m_viewPane) m_viewPane->removeAllChildren();

			int i = 0;
			while (bi.hasMore()) {
				ServerDescription s(bi);
				m_serverDescriptions.push_back(s);
				if (m_selectedServerAddress == s.applicationAddress) m_selectedServerIndex = i;
				if (m_viewPane) {
					m_serverButtons.push_back(m_viewPane->addRadioButton(s.displayText(), i, &m_selectedServerIndex));
				}
				i++;
			}
		}
	}

}

void Client::queryDiscovery() {
	if (!m_serviceDiscoveryConnection) return;

	// crude method, block until connected
	while (m_serviceDiscoveryConnection->status() == NetConnection::WAITING_TO_CONNECT) {
		System::sleep(1);
	}

	BinaryOutput bo;
	m_clientQuery.serialize(bo);
	m_serviceDiscoveryConnection->send(CLIENT_QUERY_TYPE, bo, Discovery::CHANNEL);
}

bool Client::onEvent(const GEvent& e) {
	if (GuiWindow::onEvent(e)) {
		return true;
	}

	if (e.type == GEventType::GUI_ACTION) {
		for(int i = 0; i < m_serverButtons.size(); i++){
			if (e.gui.control == m_serverButtons[i]) {
				m_selectedServerIndex = i;
				m_selectedServerAddress = m_serverDescriptions[i].applicationAddress; 
				return true;
			}
		}
	}
	return false;
}

/////////////////////////////////////////////////////////////
ServiceDiscovery::ServiceDiscovery(const shared_ptr<NetServer>& net, const Settings& settings,
	const shared_ptr<GuiTheme>& theme) :
	GuiWindow("Servers Available", theme,
		Rect2D::xywh(10, 10, windowWidth, windowHeight), GuiTheme::TOOL_WINDOW_STYLE, NO_CLOSE),
	m_settings(settings),
	m_net(net) {
	GuiScrollPane* scrollPane = pane()->addScrollPane(true, false, GuiTheme::BORDERLESS_SCROLL_PANE_STYLE);
	scrollPane->moveBy(-6, 0);
	scrollPane->setSize(windowWidth - 10, windowHeight - 10);
	m_viewPane = scrollPane->viewPane();
	m_viewPane->setWidth(windowWidth - 20);
}

shared_ptr<ServiceDiscovery> ServiceDiscovery::createNoGui(const shared_ptr<NetServer>& net, const Settings& settings) {
	return std::shared_ptr<ServiceDiscovery>(new ServiceDiscovery(net, settings));
}


shared_ptr<ServiceDiscovery> ServiceDiscovery::create(const shared_ptr<NetServer>& net,
	const shared_ptr<GuiTheme>& theme,
	const Settings& settings) {
	return std::shared_ptr<ServiceDiscovery>(new ServiceDiscovery(net, settings, isNull(theme) ? GuiTheme::lastThemeLoaded.lock() : theme));
}

void ServiceDiscovery::onNetwork() {
	if (isNull(m_net)) {
		return;
	}

	// See if there are any new connections
	// These ones won't have messages in the channel we're looking for, so we'll have to wait
	for (NetConnectionIterator& client = m_net->newConnectionIterator(); client.isValid(); ++client) {
		m_newConnections.push_back(client.connection());
		logPrintf("Discovery::ServiceDiscovery: Connection at %s", client.connection()->address().toString().c_str());
	}

	// process new connections
	for (int c = 0; c < m_newConnections.size(); ++c) {
		if (processNewConnection(m_newConnections[c])) {
			m_newConnections.fastRemove(c);
			--c;
		}
	}

	for (int c = 0; c < m_serverConnections.size(); ++c) {
		switch (m_serverConnections[c].serverConnection->status()) {

		case NetConnection::CONNECTED:
			for (NetMessageIterator& msg = m_serverConnections[c].serverConnection->incomingMessageIterator(Discovery::CHANNEL); msg.isValid(); ++msg)
			{
				if (msg.type() == SERVER_DESCRIPTION_TYPE) receiveDescription(m_serverConnections[c], msg);
			}
			break;

		case NetConnection::DISCONNECTED:
		{
			// Remove this connection from the array
			String toRemoveName = m_serverConnections[c].serverDesc.applicationName;
			if (m_viewPane) m_viewPane->remove(m_serverConnections[c].guiLabel);
			m_serverConnections.fastRemove(c);
			sendDescriptionsAll(toRemoveName);
			--c;
		}
		break;

		default:	
			break;
		} // Switch status
	} // For each server connection

	for (int c = 0; c < m_clientConnections.size(); ++c) {

		switch (m_clientConnections[c].clientConnection->status()) {

		case NetConnection::CONNECTED:
			for (NetMessageIterator& msg = m_clientConnections[c].clientConnection->incomingMessageIterator(Discovery::CHANNEL); msg.isValid(); ++msg)
			{
				if (msg.type() == CLIENT_QUERY_TYPE) updateClientQuery(m_clientConnections[c], msg);
			}
			break;

		case NetConnection::DISCONNECTED:
			// Remove this connection from the array
			m_clientConnections.fastRemove(c);
			--c;
			break;

		default:
			break;
		} // Switch status
	} // For each client connection
}

void ServiceDiscovery::sendDescriptionsToClient(const ClientData& client) const {
	BinaryOutput bo;
	for (const ServerData& server : m_serverConnections) {
		if (server.serverDesc.applicationName != client.clientQuery.applicationName || client.clientConnection->address().ip() == server.serverConnection->address().ip()) continue;

		server.serverDesc.serialize(bo);
	}
	client.clientConnection->send(SERVER_DESCRIPTION_TYPE, bo, Discovery::CHANNEL);
}

void ServiceDiscovery::sendDescriptionsAll(String& appName) const {
	//only send descriptions to applications with the same names
	for (const ClientData& client : m_clientConnections) {
		if (client.clientQuery.applicationName != appName) continue;
		sendDescriptionsToClient(client);
	}
}

bool ServiceDiscovery::processNewConnection(const shared_ptr<NetConnection>& connection) {
	/*
	Read all incoming messages from all connections, regardless of who created them.
	New Connections can only be sorted when the first message is sent, so they linger in
	the new connections array until then.
	*/
	for (NetMessageIterator& msg = connection->incomingMessageIterator(Discovery::CHANNEL); msg.isValid(); ++msg) {
		switch (msg.type()) {
		case CLIENT_QUERY_TYPE:
			m_clientConnections.append(ClientData{ ClientQuery(), connection });
			return true;

		case SERVER_DESCRIPTION_TYPE:
			m_serverConnections.append(ServerData{ ServerDescription(), connection , nullptr });
			return true;
		} // Dispatch on message type
	} // For each message
	return false;
}

void ServiceDiscovery::receiveDescription(ServerData& server, NetMessageIterator& msg) {
	ServerDescription s(msg.binaryInput());
	s.lastUpdateTime = System::time();
	server.serverDesc = s;
	if (m_viewPane) {
		m_viewPane->remove(server.guiLabel);
		server.guiLabel = m_viewPane->addLabel(server.serverDesc.displayText());
		server.guiLabel->setWidth(windowWidth - 30);
		m_viewPane->pack();
	}
	sendDescriptionsAll(server.serverDesc.applicationName);
}

void ServiceDiscovery::updateClientQuery(ClientData& client, NetMessageIterator& msg) {
	client.clientQuery = ClientQuery(msg.binaryInput());
	sendDescriptionsToClient(client);
}

//////////////////////////////////////////////////////////////////////////////////////////////

shared_ptr<Server> Server::create(const ServerDescription& description,
	const Settings& settings) {
	return std::make_shared<Server>(Server(description, settings));
}


Server::Server(const ServerDescription& description, const Settings& settings) :
	m_settings(settings),
	m_description(description) {

	for (NetAddress addr : m_settings.serviceDiscoveryAddresses) {
		try {
			m_serviceDiscoveryConnection = NetConnection::connectToServer(addr);
			break;
		}
		catch (...) {
			logPrintf("Discovery::Server: Service Discovery at %s not found", addr.toString().c_str());
		}
	}
	sendDescriptionToDiscovery();
}

void Server::setDescription(const ServerDescription& d) {
	m_description = d;
	sendDescriptionToDiscovery();
}


void Server::sendDescriptionToDiscovery() {
	// Send to all clients
	if (!m_serviceDiscoveryConnection) return;

	BinaryOutput bo;
	m_description.serialize(bo);

	// crude method, block until connected
	while (m_serviceDiscoveryConnection->status() == NetConnection::WAITING_TO_CONNECT) {
		System::sleep(1);
	}
	m_serviceDiscoveryConnection->send(SERVER_DESCRIPTION_TYPE, bo, Discovery::CHANNEL);
	m_lastAdvertisementTime = System::time();
}

void Server::onNetwork() {
	if (isNull(m_serviceDiscoveryConnection)) {
		return;
	}

	if (m_serviceDiscoveryConnection->status() == NetConnection::DISCONNECTED) {
		m_serviceDiscoveryConnection = nullptr;
		logPrintf("Discovery::Server: Service Discovery disconnected");
	}
}
} // Discovery


} // G3D

