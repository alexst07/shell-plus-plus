# execute simple cmds
# --output:start
# true
# not
t = $(echo ok)

if t {
  print("true")
}

assert(t.status() == 0, "error")

t = $(echo ok | cat)
assert(t.status() == 0, "error")

cmd m {
  echo teste
  exit 0
}

t = $(m | cat)
assert(t.status() == 0, "error")

cmd err {
  exit 1
}

t = $(err)
if !t {
  print("not")
}

assert(t.status() != 0, "error")
