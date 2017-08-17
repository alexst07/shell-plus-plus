# interfaces tests
# --output:start
# Print 2:3
# Ftest 7:8
# Fother 9
# --output:end

interface ITest {
  func Print(a, b)
}

interface IOther {
  func Fother(a)
}

interface ITest2: IOther {
  func Ftest(x, y)
}

class Test: ITest, ITest2 {
  func Print(a, b = 3) {
    print("Print ", a, ":", b)
  }

  func Ftest(x, y) {
    print("Ftest ", x, ":", y)
  }

  func Fother(x) {
    print("Fother ", x)
  }
}

t = Test()
t.Print(2)
t.Ftest(7, 8)
t.Fother(9)
