# inner class test
# --output:start
# >> THIS A: 15
# >> THIS B: 11
# >> THIS C: 12
# --output:end

class Test1 {
  func __init__(a) {
    this.a = a
  }

  func dump() {
    dump_symbol_table()
  }

  func Print() {
    print(">> THIS A: ", this.a)
  }

  class Test2 {
    func __init__(b) {
      this.b = b
    }

    func Print() {
      print(">> THIS B: ", this.b)
    }

    func dump() {
      dump_symbol_table()
    }

    class Test3 {
      func __init__(b) {
        this.c = b
      }

      func Print() {
        print(">> THIS C: ", this.c)
      }

      func dump() {
        dump_symbol_table()
      }
    }
  }
}


t = Test1(15)
t.Print()
t1 = Test1.Test2(11)
t1.Print()
t2 = Test1.Test2.Test3(12)
t2.Print()
