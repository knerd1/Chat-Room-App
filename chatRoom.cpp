#include "chatRoom.hpp"
#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/detail/is_buffer_sequence.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/detail/error_code.hpp>
#include <cstddef>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <utility>

using namespace std;

void Room::join(participantPointer
                    participant) { // when client joins, it should join a room.
  this->participants.insert(participant);
}

void Room::leave(participantPointer participant) {
  this->participants.erase(participant);
}

void Room::deliver(participantPointer participant, Message &message) {
  messageQueue.push_back(
      message); // now, whenever it wants to start its delivery it call call
                // start() function, where it will listen for incoming messages
                // and push to the message Queue of the room
  while (!messageQueue.empty()) {
    Message msg = messageQueue.front();
    messageQueue.pop_front();

    for (participantPointer _paticipants : participants) {
      if (participant != _paticipants) {
        _paticipants->write(msg);
      }
    }
  }
}

void Session ::async_read() {
  auto self(shared_from_this());
  boost::asio::async_read_until(
      clientSocket, buffer, "\n",
      [this, self](boost::system::error_code ec, size_t bytes_transferred) {
        if (!ec) {
          string data(boost::asio::buffers_begin(buffer.data()),
                      boost::asio::buffers_begin(buffer.data()) +
                          bytes_transferred);
          buffer.consume(bytes_transferred);
          cout << "Recieved: " << data << endl;
          Message message(data);
          deliver(message);
          async_read();
        } else {
          room.leave(shared_from_this());
          if (ec == boost::asio::error::eof) {
            cout << "Connection closed by peer" << endl;
          } else {
            cout << "Read error" << ec.message() << endl;
          }
        }
      });
}

void Session::async_write(string messageBody, size_t MessageLength) {
  auto write_handler = [&](boost::system::error_code ec,
                           size_t bytes_transferred) {
    if (!ec) {
      cout << "Data is written to the socket" << endl;
    } else {
      cerr << "write error: " << ec.message() << endl;
    }
  };
  boost::asio::async_write(clientSocket,
                           boost::asio::buffer(messageBody, MessageLength),
                           write_handler);
}

void Session::start() {
  room.join(shared_from_this());
  async_read();
}

Session::Session(tcp::socket s,
                 Room &r) // hence a session(socket, room) will be created and
                          // this client will have a session
    : clientSocket(std::move(s)), room(r) {};

void Session::write(
    Message &message) { // room will call write() function to write any message
                        // to the client's queue It will trigger the write() for
                        // each participant except the sender itself
  messageQueue.push_back(message);
  while (messageQueue.size() != 0) {
    Message message = messageQueue.front();
    messageQueue.pop_front();
    bool header_decode = message.decodeHeader();
    if (header_decode) {
      string body = message.getBody();
      async_write(body, message.getNewBodyLength());
    } else {
      cout << "Message-length exceeds the max allowed length" << endl;
    }
  }
}

void Session::deliver(
    Message &message) { // when client wants to send message it can call
                        // session's deliver() message session will call
                        // deliver() to deliver the message to the room
  room.deliver(shared_from_this(), message);
}
// data stores the header+bodyLength with maximum size of header+maxBytes

using boost::asio::ip::address_v4;

void accecpt_connection(boost::asio::io_context &io,
                        char *port, // Its async accepting Connection
                        tcp::acceptor &acceptor, Room &room,
                        const tcp::endpoint &endpoint) {
  tcp::socket socket(io);
  acceptor.async_accept([&](boost::system::error_code ec,
                            tcp::socket socket) { // Lambda function that
                                                  // works as inline function
    if (!ec) {
      shared_ptr<Session> session =
          make_shared<Session>(std::move(socket), room);
      session->start();
    }
    accecpt_connection(io, port, acceptor, room, endpoint);
  });
}

int main(int argc, char *argv[]) {
  try {
    if (argc < 2) {
      cerr << "Usage: Server <port>";
      return 1;
    }
    Room room;
    boost::asio::io_context io_context;
    tcp::endpoint endpoint(tcp::v4(), atoi(argv[1]));
    tcp::acceptor acceptor(io_context, endpoint);
    accecpt_connection(io_context, argv[1], acceptor, room, endpoint);
    io_context.run();
  } catch (exception &e) {
    cerr << "Exception: " << e.what() << endl;
  }
  return 0;
}
