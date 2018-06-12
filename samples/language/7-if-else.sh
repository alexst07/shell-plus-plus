a = 5
b = 8

if a < b {
  print("a < b -> true")
} else {
  print("a < b -> false")
}

t = "test"

if t == "other" {
  print("t == other")
} else if t == "tests" {
  print("t == tests")
} else {
  print("none above")
}

x = true
y = true

if x && y {
  print("x && y -> ", true)
}

# testing compration operators
print("7 == 7 -> ", 7 >= 7)
print("7 != 17 -> ", 7 != 17)
print("7 >= 7 -> ", 7 >= 7)
print("7 <= 7 -> ", 7 <= 7)
print("9 > 8 -> ", 9 > 8)
print("7 < 8 -> ", 7 < 8)
print("true && true -> ", true && true)
print("true || false -> ", true || false)

# ! and 'not' has different precedence
print("!false && true -> ", !false && true)
print("!(true && false) -> ", !(true && false))
print("! true && true -> ", ! true && true)
print("not false && false -> ", not false && false)
print("not not not !!!!false -> ", not not not !!!!false)

# bitwise operators
print("7 & 3 -> ", 7 & 3)
print("7 | 3 -> ", 7 | 3)
print("7 ^ 3 -> ", 7 ^ 3)
print("~5 -> ", ~5)
