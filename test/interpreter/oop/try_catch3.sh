# interfaces tests
# --output:start
# asdf
# <type: InvalidCmdException>
# Exception: zXxers: No such file or directory
# finally
# --output:end

try {
  echo asdf
  zXxers
  echo fdsa
} catch GlobException as t {
  print(string(t))
} catch Exception as t {
  print(type(t))
  print("Exception: " + string(t))
} finally {
  print("finally")
}
