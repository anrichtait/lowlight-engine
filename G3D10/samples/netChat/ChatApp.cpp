#include "G3D/G3D.h"
#include <functional>

/** Chat example: p2p chat in which every node is both a client and a server */
class ChatApp : public GApp {
protected:

    enum {PORT = 18821};
    enum MessageType {TEXT = 1, CHANGE_NAME};
    const NetChannel CHAT_CHANNEL = 0;

    class Conversation : public ReferenceCountedObject {
    public:
        String                      name;
        shared_ptr<NetConnection>   connection;
        shared_ptr<GuiWindow>       window;
        GuiTextBox*                 textBox;
        GuiLabel*                   lastTextReceived;
        String                      textToSend;

    protected:

        Conversation(ChatApp* app, const shared_ptr<NetConnection>& c) : connection(c) {
            window = GuiWindow::create("New Connection", shared_ptr<GuiTheme>(), Rect2D::xywh(100, 100, 100, 100), GuiTheme::NORMAL_WINDOW_STYLE, GuiWindow::REMOVE_ON_CLOSE);
            // One line of history!
            lastTextReceived = window->pane()->addLabel("");
            textBox = window->pane()->addTextBox("", &textToSend);
            app->addWidget(window);
        }

    public:

        static shared_ptr<Conversation> create(ChatApp* app, const shared_ptr<NetConnection>& c) {
            return createShared<Conversation>(app, c);
        }

        ~Conversation() {
            connection->disconnect(false);
        }
    };

    String                                  m_name;

    /** For finding others and letting others find me */
    shared_ptr<Discovery::Server>         m_discoveryServer;
    shared_ptr<Discovery::Client>         m_discoveryClient;

    /** For allowing others to connect to me */
    shared_ptr<NetServer>                   m_server;

    /** All of my connections, regardless of who created them */    
    Array< shared_ptr<Conversation> >       m_conversationArray;

    /** Returns a null pointer if there is no matching conversation. */
    shared_ptr<Conversation> findConversation(const GuiControl* c) {
        for (int i = 0; i < m_conversationArray.size(); ++i) {
            if (m_conversationArray[i]->textBox == c) {
                return m_conversationArray[i];
            }
        }
        return shared_ptr<Conversation>();
    }

    shared_ptr<Conversation> findConversation(const NetAddress& addr) {
        for (int i = 0; i < m_conversationArray.size(); ++i) {
            if (m_conversationArray[i]->connection->address() == addr) {
                return m_conversationArray[i];
            }
        }
        return shared_ptr<Conversation>();
    }

    void sendMyName(shared_ptr<NetSendConnection> connection) const {
        BinaryOutput bo;
        bo.writeString32(m_name);
        connection->send(CHANGE_NAME, bo);
    }

    void connectToServer(const NetAddress& address) {
        m_conversationArray.append(Conversation::create(this, NetConnection::connectToServer(address)));
    }

public:

    ChatApp(const GApp::Settings& s) : GApp(s) {}

    void onInit() override {
        GApp::onInit();

        renderDevice->setColorClearValue(Color3::white());

        m_server = NetServer::create(PORT);

        m_discoveryServer = Discovery::Server::create(Discovery::ServerDescription(PORT));
        logPrintf("ChatApp: Server at %s", NetAddress::localHostname().c_str());

        m_discoveryClient = Discovery::Client::create();

        developerWindow->setVisible(false);
        developerWindow->cameraControlWindow->setVisible(false);
        developerWindow->sceneEditorWindow->setVisible(false);
        showRenderingStats = false;
        debugWindow->setVisible(false);

        window()->setCaption(" (" + NetAddress::localHostname() + ":" + format("%d", PORT) + ")");
        addWidget(m_discoveryServer);
        addWidget(m_discoveryClient);
    }

    // overloaded differentiation so that we're not looking through conversations every time a GUI_ACTION happens
    void maybeStartConversation(GuiControl *t) {} 
    
    void maybeStartConversation(GuiRadioButton *t) {
        shared_ptr<Conversation> conversation = findConversation(m_discoveryClient->selectedServerAddress());
        if (isNull(conversation)) {
            connectToServer(m_discoveryClient->selectedServerAddress());
        }
    } 

    virtual bool onEvent(const GEvent& e) override {
        if (GApp::onEvent(e)) {
            //GApp::onEvent calls onEvent for widgets. If it returns true then action might've happened with m_discoveryClient 
            if (e.type == GEventType::GUI_ACTION) maybeStartConversation(e.gui.control);
            return true;
        }

        switch (e.type) {
        case GEventType::GUI_ACTION:
            {
                // Send a text message to the other machine
                shared_ptr<Conversation> conversation = findConversation(e.gui.control);
                if (notNull(conversation)) {
                    BinaryOutput bo;
                    bo.writeString32(conversation->textToSend);
                    conversation->connection->send(TEXT, bo);
                    conversation->textToSend = "";
                    return true;
                }

            }
            break;

        case GEventType::GUI_CLOSE:
            {
                // Shut down the associated network connection by letting 
                // the Conversation's destructor execute
                for (int i = 0; i < m_conversationArray.size(); ++i) {
                    if (m_conversationArray[i]->window.get() == e.guiClose.window) {
                        m_conversationArray.fastRemove(i);
                        break; // Leave the FOR loop
                    }
                } // for
                System::sleep(0.5);
            }
            break;

        case GEventType::QUIT:
            m_conversationArray.clear();
            m_server->stop();
            System::sleep(1);
            m_server.reset();
            // Let the connection close on the thread before shutting down
            System::sleep(2);
            exit(0); 
            break;
        } //switch

        return false;
    } // onEvent


    void onNetwork() override {
        // If the app is shutting down, don't service network connections
        if (isNull(m_server)) {
            return;
        }

        // For widgets
        GApp::onNetwork(); 

        // See if there are any new clients
        for (NetConnectionIterator& client = m_server->newConnectionIterator(); client.isValid(); ++client) {
            m_conversationArray.append(Conversation::create(this, client.connection()));

            // Update Service Discovery
            Discovery::ServerDescription newDescription = m_discoveryServer->description();
            newDescription.currentClients++;
            m_discoveryServer->setDescription(newDescription);
        }

        for (int c = 0; c < m_conversationArray.size(); ++c) {
            const shared_ptr<Conversation>& conversation = m_conversationArray[c];

            switch (conversation->connection->status()) {
            case NetConnection::WAITING_TO_CONNECT:
                // Still waiting for the server to accept us.
                break;

            case NetConnection::JUST_CONNECTED:
                // We've just connected to the server but never invoked send() or incomingMessageIterator().
                // Tell the server our name.
                sendMyName(conversation->connection);

                // Intentionally fall through

            case NetConnection::CONNECTED:
                // Read all incoming messages from all connections, regardless of who created them
                for (NetMessageIterator& msg = conversation->connection->incomingMessageIterator(CHAT_CHANNEL); msg.isValid(); ++msg) {

                    BinaryInput& bi = msg.binaryInput();

                    switch (msg.type()) {
                    case TEXT:
                        conversation->lastTextReceived->setCaption(bi.readString32());
                        break;

                    case CHANGE_NAME:
                        conversation->name = bi.readString32();
                        conversation->window->setCaption(conversation->name);
                        break;
                    } // Dispatch on message type

                } // For each message
                break;

            case NetConnection::DISCONNECTED:
                // Remove this conversation from my list
                removeWidget(m_conversationArray[c]->window);
                m_conversationArray.fastRemove(c);
                --c;
                break;

            case NetConnection::WAITING_TO_DISCONNECT:
                break;
            } // Switch status
        } // For each conversation
    }
}; // class ChatApp


void runChat(const GApp::Settings& settings) {
}

// Tells C++ to invoke command-line main() function even on OS X and Win32.
G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    initGLG3D();

    GApp::Settings settings(argc, argv);
    
    return ChatApp(settings).run();
}
