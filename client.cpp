//
// Created by hiro on 1/10/24.
//

//#include "Client.h"
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>

#include <enet/enet.h>

ENetHost* client;
ENetPeer* peer;
ENetEvent event;
std::condition_variable cv;
bool disconnectFlag = false;
std::mutex mutex;

void ReceiveMessages() {
    while (true) {
        while (enet_host_service(client, &event, 1000) > 0) {
            switch (event.type) {
                case ENET_EVENT_TYPE_CONNECT:
                    std::cout << "Connected to the server." << std::endl;
                    break;

                case ENET_EVENT_TYPE_RECEIVE:
                    std::cout << "Received (echo): " << event.packet->data << std::endl;
                    break;

                case ENET_EVENT_TYPE_DISCONNECT:
                    std::cout << "Disconnected from the server." << std::endl;
                    return;
                    break;

                default:
                    break;
            }
        }
    }
}

int main() {
    if (enet_initialize() != 0) {
        std::cerr << "Failed to initialize ENet." << std::endl;
        return EXIT_FAILURE;
    }

    client = enet_host_create(nullptr, 1, 2, 0, 0);

    std::cout<< "1" << std::endl;

    if (client == nullptr) {
        std::cerr << "Failed to create ENet client." << std::endl;
        enet_deinitialize();
        return EXIT_FAILURE;
    }

    ENetAddress address;
    enet_address_set_host(&address, "127.0.0.1");
    address.port = 30080;

    // 초기 연결 시도
    peer = enet_host_connect(client, &address, 2, 0);

    std::cout<< "2" << std::endl;

    if (peer == nullptr) {
        std::cerr << "No available peers for connection." << std::endl;
        enet_host_destroy(client);
        enet_deinitialize();
        return EXIT_FAILURE;
    }

    std::cout<< "3" << std::endl;

    // 메시지 수신을 담당할 스레드 시작
    std::thread receiveThread(ReceiveMessages);

    std::string message;
    while (true) {
        std::cout << "Enter message: ";
        std::getline(std::cin, message);

        if (message == "exit") {
            std::cout << "okay bye" << std::endl;
            enet_peer_disconnect(peer, 0);
            disconnectFlag = true;  // 종료 플래그 설정
            cv.notify_one();        // 메시지 수신 스레드에 종료 플래그 알림
            break;
        }

        // 메시지를 서버로 전송
        ENetPacket* packet = enet_packet_create(message.c_str(), message.length() + 1, ENET_PACKET_FLAG_RELIABLE);
        enet_peer_send(peer, 0, packet);
        std::cout << "send Data: " << packet->data << std::endl;
    }

    std::cout<< "4" << std::endl;
    std::unique_lock<std::mutex> lock(mutex);
    cv.wait_for(lock, std::chrono::milliseconds(3000), [] { return disconnectFlag; });
    receiveThread.join();

//    enet_peer_disconnect(peer, 0);
    while (enet_host_service(client, &event, 1000) > 0) {
        switch (event.type) {
            case ENET_EVENT_TYPE_RECEIVE:
                enet_packet_destroy(event.packet);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                std::cout << "Disconnected from the server." << std::endl;
                break;

            default:
                break;
        }
    }
    std::cout<< "5" << std::endl;
    enet_host_destroy(client);
    enet_deinitialize();

    return EXIT_SUCCESS;
}