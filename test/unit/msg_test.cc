#include <sstream>
#include <gtest/gtest.h>

#include "msg.h"

TEST(Message, ErrorSeverityIsIndependentOfConstness) {
  using shpp::internal::Message;
  Message message(Message::Severity::ERR, boost::format("synthetic failure"), 2, 3);
  std::ostringstream mutable_output;
  std::ostringstream const_output;

  mutable_output << message;
  const_output << static_cast<const Message&>(message);

  EXPECT_EQ("2:3: error: synthetic failure\n", mutable_output.str());
  EXPECT_EQ(const_output.str(), mutable_output.str());
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
