#ifndef SETTI_MSG_H
#define SETTI_MSG_H

#include <string>
#include <memory>
#include <vector>
#include <boost/format.hpp>

namespace setti {
namespace internal {

class Message {
 public:
  enum class Severity {
    INFO,
    WARNING,
    ERR
  };

  Message() = delete;

  Message(Severity severity, const boost::format& msg, uint line, uint pos)
      : severity_(severity)
      , msg_(msg)
      , line_(line)
      , pos_(pos) {}

  friend std::ostream& operator<<(std::ostream& stream, const Message& msg);
  friend std::ostream& operator<<(std::ostream& stream, Message& msg);

 private:
  uint line_;
  uint pos_;
  boost::format msg_;
  Severity severity_;
};

inline std::ostream& operator<<(std::ostream& stream, const Message& msg) {
  std::string severity;

  switch (msg.severity_) {
    case Message::Severity::INFO:
      severity = "info";
      break;

    case Message::Severity::WARNING:
      severity = "warning";
      break;

    case Message::Severity::ERR:
      severity = "error";
      break;
  }

  stream << msg.line_ << ":" << msg.pos_ << ": " << severity << ": " << msg.msg_ << '\n';

  return stream;
}

inline std::ostream& operator<<(std::ostream& stream, Message& msg) {
  std::string severity;

  switch (msg.severity_) {
    case Message::Severity::INFO:
      severity = "info";
      break;

    case Message::Severity::WARNING:
      severity = "warning";
      break;

    case Message::Severity::ERR:
      severity = "info";
      break;
  }

  stream << msg.line_ << ":" << msg.pos_ << ": " << severity << ": " << msg.msg_ << '\n';

  return stream;
}

class Messages {
 public:
  using iterator = std::vector<Message>::iterator;
  using const_iterator = std::vector<Message>::const_iterator;

  Messages() = default;

  Messages(Messages&& msg):msg_vec_(std::move(msg.msg_vec_)) {}

  Messages& operator=(Messages&& msg) {
    msg_vec_ = std::move(msg.msg_vec_);
    return *this;
  }

  Messages(const Messages& msg):msg_vec_(msg.msg_vec_) {}

  Messages& operator=(const Messages& msg) {
    msg_vec_ = msg.msg_vec_;
    return *this;
  }

  void Push(Message&& msg) {
    msg_vec_.push_back(std::move(msg));
  }

  iterator begin() {
    return msg_vec_.begin();
  }

  iterator end() {
    return msg_vec_.end();
  }

  const_iterator begin() const {
    return msg_vec_.begin();
  }

  const_iterator end() const {
    return msg_vec_.end();
  }

 private:
  std::vector<Message> msg_vec_;
};

}
}

#endif  // SETTI_MSG_H

