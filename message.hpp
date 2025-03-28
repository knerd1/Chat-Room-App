#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string.h>
#include <string>

using namespace std;

#ifndef MESSAGE_HPP
#define MESSAGE_HPP

class Message {

public:
  Message() : bodyLength_(0) {}
  enum {
    maxBytes = 512
  }; // header is of 4 bytes and maxBytes can be stored as 512 bytes, header
     // stores the body length that is the current body length
  enum { header = 4 };

  Message(string message) {
    bodyLength_ = getNewBodyLength(message.size());
    encodeHeader();
    std::memcpy(data + header, message.c_str(),
                bodyLength_); // data stores the header+bodyLength with maximum
                              // size of header+maxBytes
  }

  size_t getNewBodyLength(size_t newLength) {
    if (newLength > maxBytes) {
      return maxBytes;
    }
    return newLength;
  }

  string getData() {
    int lenght = header + bodyLength_;
    string result(data, lenght);
    return result;
  }

  string getBody() {
    string dataStr = getData();
    string result = dataStr.substr(header, bodyLength_);
    return result;
  }

  bool decodeHeader() {
    char new_header[header + 1] = "";
    strncpy(new_header, data, header);
    new_header[header] = '\0';
    int headervalue = atoi(new_header);
    if (headervalue > maxBytes) {
      bodyLength_ = 0;
      return false;
    }
    bodyLength_ = headervalue;
    return true;
  }

  void encodeHeader() {
    char new_header[header + 1] = "";
    sprintf(new_header, "%4d", static_cast<int>(bodyLength_));
    memcpy(data, new_header, header);
  }

  size_t getNewBodyLength() { return bodyLength_; }

private:
  char data[header + maxBytes];
  size_t bodyLength_; // Represent data Lenght Which can not be negative(use
                      // size_t)...
};

#endif MESSAGE_HPP

// client attempts to send message:- It will encode header and put message into
// the data and send data server gets the message, decodes the header, get the
// bodylength from the header and hence complete body then server sends the
// message to all the clients connected to that room.
