# execute decl command
# --output:start
# gone.txt
# go.txt
# --output:end

cmd test {
  mkdir dcl_teste
  touch dcl_teste/go.txt
  touch dcl_teste/gone.txt
  touch dcl_teste/ok.txt

  ls dcl_teste | grep go

  rm -rf dcl_teste
}

test
