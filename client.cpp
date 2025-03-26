#include "message.hpp"
#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/impl/write.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/system/detail/error_code.hpp>
#include <future>
#include <iostream>
#include <istream>
#include <memory>
#include <string>

using namespace std;

using boost::asio::ip::tcp;

void async_read(tcp::socket &socket) {
  auto buffer = std::make_shared<boost::asio::streambuf>();
  boost::asio::async_read_until(
      socket, *buffer, "\n",
      [&socket, buffer](boost::system::error_code ec, size_t length) {
        if (!ec) {
          istream is(buffer.get());
          string received;
          getline(is, received);
          cout << "Server: " << received << endl;
          async_read(socket);
        }
      });
}

int main(int argc, char *argv[]) {
  if (argc < 2) {
    cerr << "Provide Port too as a second argument" << endl;
    return 1;
  }
  boost::asio::io_context io_context;
  tcp::socket socket(io_context);
  tcp::resolver resolver(io_context);

  boost::asio::connect(socket, resolver.resolve("127.0.0.1", argv[1]));
  async_read(socket);
  thread t([&io_context, &socket]() {
    while (true) {
      string data;
      cout << "Enter message: ";
      getline(cin, data);
      data += "\n";
      boost::asio::post(io_context, [&, data]() {
        boost::asio::write(socket, boost::asio::buffer(data));
      });
    }
  });
  io_context.run();
  t.join();
  return 0;
}
