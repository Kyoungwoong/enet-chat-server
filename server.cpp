//
// Created by hiro on 1/10/24.
//

//#include "server.h"
#include <iostream>
#include <cstring>
#include <enet/enet.h>

ENetHost* server = nullptr;

enum TYPE {
    CONNECT, DISCONNECT
};

// 채팅 메시지를 처리하는 함수
void HandleChatMessage(ENetPeer* sender, const char* message) {
    // 채팅 메시지를 받은 클라이언트의 주소와 메시지 내용을 출력 또는 처리
    printf("Received message from %u.%u.%u.%u: %s\n",
           sender->address.host & 0xFF, (sender->address.host >> 8) & 0xFF,
           (sender->address.host >> 16) & 0xFF, (sender->address.host >> 24) & 0xFF,
           message);

    // 받은 메시지를 모든 클라이언트에게 브로드캐스트
    ENetPacket* broadcastPacket = enet_packet_create(message, strlen(message) + 1, ENET_PACKET_FLAG_RELIABLE);
    enet_host_broadcast(server, 0, broadcastPacket);
}

// 패킷 처리 함수
void ProcessPacket(ENetPeer* sender, const ENetPacket* packet) {

    // 채팅 메시지 패킷 처리
    const char* message = (const char*)(packet->data);

    HandleChatMessage(sender, message);
}

void DisplayClientInfo(ENetPeer* sender, TYPE state) {
    char buffer[100];

    switch (state) {
        case CONNECT:
            printf("Request Connection about Client from %x\n", sender->address.host);
            sprintf(buffer, "Enter client %x", sender->address.host);
            break;

        case DISCONNECT:
            printf("Request Disconnection about client from %x\n", sender->address.host);
            sprintf(buffer, "Exit client %x", sender->address.host);
            break;
    }
    // 브로드캐스트할 패킷 생성
    ENetPacket* broadcastPacket = enet_packet_create(buffer, strlen(buffer) + 1, ENET_PACKET_FLAG_RELIABLE);

    // ENET_PACKET_FLAG_RELIABLE은 패킷의 전송을 보장합니다.
    enet_host_broadcast(server, 0, broadcastPacket);
}

int main() {
    if (enet_initialize() != 0) {
        std::cerr << "ENet initialization failed!" << std::endl;
        return EXIT_FAILURE;
    }

    ENetAddress address;

    address.host = enet_address_set_host(&address, "127.0.0.1");
    address.port = 30080;

    server = enet_host_create(&address, 32, 2, 0, 0);

    if (server == nullptr) {
        std::cerr << "ENet server creation failed!" << std::endl;
        enet_deinitialize();
        return EXIT_FAILURE;
    }

    std::cout << "ENet server started on port 30080..." << std::endl;

    ENetEvent event;
    while (true) {
        if (enet_host_service(server, &event, 0) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    std::cout << "Client connected." << std::endl;
                    DisplayClientInfo(event.peer, CONNECT);
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    std::cout << "Received: " << event.packet->data << std::endl;
                    ProcessPacket(event.peer, event.packet);
                    enet_packet_destroy(event.packet);
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Client disconnected." << std::endl;
                    DisplayClientInfo(event.peer, DISCONNECT);
                    break;

                default:
                    break;
            }
        }
    }

    enet_host_destroy(server);
    enet_deinitialize();

    return EXIT_SUCCESS;
}

