# while test
# --output:start
# cmd:  teste var
# inside cmd:  teste var
# cmd:  cmd teste var
# inside cmd:  cmd teste var
# varenv: teste var
# cmd:  change teste var
# inside cmd:  change teste var
# cmd:  cmd teste var
# inside cmd:  cmd teste var
# varenv: change teste var
# --output:end

cmd test {
  echo "cmd: " ${$TESTVAR}
  inside_test
  varenv TESTVAR = "cmd teste var"
  echo "cmd: " ${$TESTVAR}
  inside_test
}

cmd inside_test {
  echo "inside cmd: " ${$TESTVAR}
}

varenv TESTVAR = "teste var"
test
print("varenv: ", $TESTVAR)

varenv TESTVAR = "change teste var"
test
print("varenv: ", $TESTVAR)
