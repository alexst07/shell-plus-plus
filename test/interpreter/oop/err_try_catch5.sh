try {
  echo asdf
  zXxers
  echo fdsa
} catch GlobException as t {
  print(string(t))
} catch BadAllocException as t {
  print(type(t))
  print("Exception: " + string(t))
} finally {
  print("finally")
}
