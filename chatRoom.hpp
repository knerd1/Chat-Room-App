#include "message.hpp"
#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <deque>
#include <iostream>
#include <locale>
#include <memory>
#include <set>
#include <sys/socket.h>
#include <unistd.h>

#ifndef CHATROOM_HPP
#define CHATROOM_HPP

using namespace std;
using boost::asio::ip::tcp; // tcp coming from boost library within namespace
                            // asio,ip

class Participants {

public:
  virtual void deliver(Message &message) = 0;
  virtual void write(Message &message) = 0;
  virtual ~Participants() = default;
};

typedef shared_ptr<Participants>
    participantPointer; // Shared pointer to use, whenever you want to define
                        // ownership

class Room {
public:
  void join(participantPointer participant);
  void leave(participantPointer participant);
  void deliver(participantPointer participant, Message &message);

private:
  deque<Message> messageQueue;
  enum { MaxParticipants = 100 };
  set<participantPointer>
      participants; // Set for Participants Because,C2 Sending message And it
                    // goes to only c1 and c3 Participants dont come back to c2
};

class Session : public Participants,
                public enable_shared_from_this<
                    Session> { // whenever Object of this class has been created
                               // you can pass that object as a pointer to
                               // someone and move the ownership to someone else
public:
  Session(tcp::socket s, Room &room);
  void start();
  void deliver(Message &message);
  void write(Message &message);
  void async_read();
  void async_write(string messageBody, size_t messageLength);

private:
  tcp::socket clientSocket;
  boost::asio::streambuf
      buffer; // whenever reading from socket and writing to
              // socket, it needs buffer(kind of queue) where you puts chars
              // Because socket its networks based its can take time so
              // meanwhile to prevent for (sockets have not any new chars at
              // time of buffers)

  Room &room;
  deque<Message> messageQueue;
};

#endif CHATROOM_HPP
