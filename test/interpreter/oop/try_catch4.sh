# interfaces tests
# --output:start
# asdf
# Exception
# finally
# --output:end

try {
  echo asdf
  zXxers # not found command
  echo fdsa
} catch GlobException as t {
  print(string(t))
} catch Exception {
  print("Exception")
} finally {
  print("finally")
}
