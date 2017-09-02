# interfaces tests
# --output:start
# asdf
# <type: TestException>
# Exception: tessdfte
# --output:end

class TestException(Exception) {
  func __init__(msg, code) {
    Exception.__init__(this, msg)
    this.code = code
  }
}

try {
  echo asdf
  throw TestException("tessdfte", 5)
  echo fdsa
} catch GlobException as t {
  print(string(t))
} catch Exception, TestException as t {
  print(type(t))
  print("Exception: " + string(t))
}
