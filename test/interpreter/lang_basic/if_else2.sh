# if else test
# --output:start
# test
# elseif2
# elseif1
# else
# --output:end

a = 5
b = "test"

if a == 5 {
  if b != "test" {
    print("no test")
  } else {
    print("test")
  }
} else {
  print("asdf")
}

if a == 6 {
  print("if")
} else if a == 3 {
  print("elseif1")
} else if a == 5 {
  print("elseif2")
} else {
  print("else")
}

if a == 6 {
  print("if")
} else if a == 5 {
  print("elseif1")
} else if a == 3 {
  print("elseif2")
} else {
  print("else")
}

if a == 6 {
  print("if")
} else if a == 3 {
  print("elseif1")
} else if a == 4 {
  print("elseif2")
} else {
  print("else")
}
