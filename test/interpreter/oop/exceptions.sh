class TestException(Exception) {
  func __init__(msg, code) {
    Exception.__init__(this, msg)
    this.code = code
  }

  func showSymTable() {
    dump_symbol_table(this)
  }

}

t = TestException("tessdfte", 45)
print(string(t))
t.showSymTable()
