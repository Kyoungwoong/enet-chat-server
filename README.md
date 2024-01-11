# enet기반 채팅서버

---

RUDP를 기반으로 한 Enet을 이용한 채팅 서버 개발

# enet 설치

---

```
sudo apt-get update
sudo apt-get install libenet-dev
```


# 실행 

---

```
g++ -o server server.cpp -lenet
g++ -o client client.cpp -lenet
./server
./client
```
