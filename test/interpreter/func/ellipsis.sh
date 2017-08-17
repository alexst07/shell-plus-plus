# execute simple cmds
# --output:start
# case 1
# case 3
# 98alex8 teste98
# alex8 teste
# --output:end

func f(args...) {
  print(98, ...args, 98)
}

p = "alex"
v = ["alex", "outro"]

switch p {
  case ...v {
    print("case 1")
  }

  case "outro" {
    print("case 2")
  }

  case "alex" {
    print("case 3")
  }

  default {
    print("default")
  }
}

f("alex", 8, " teste")
print("alex", 8, " teste")
