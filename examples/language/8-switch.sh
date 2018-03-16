# switch statement works similar with others languages as C, C++, Java  and
# javascript, but its case blocks are delimited by {} insted of break.
#
# a case can have several expression separated by ',', it means the case
# accept a list of expression, if some these expression matches, so the case
# block a executed

t = "cat"

switch t {
  case "ls" {
    print("command ls")
  }

  case "cat", "mv" {
    print("command cat or mv")
  }

  default {
    print("none above")
  }
}

t = "other"

switch t {
  case "ls" {
    print("command ls")
  }

  case "cat", "mv" {
    print("command cat or mv")
  }

  default {
    print("none above")
  }
}

t = "ls"

switch t {
  case "ls" {
    print("command ls")
  }

  case "cat", "mv" {
    print("command cat or mv")
  }

  default {
    print("none above")
  }
}
