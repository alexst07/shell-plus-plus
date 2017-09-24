# simple class with final keyword
# --output:start
# 'Test1' can't extends from final type 'Test'
# --output:end

final class Test {}

try {
  class Test1(Test) {}
} catch Exception as ex {
  print(ex)
}
