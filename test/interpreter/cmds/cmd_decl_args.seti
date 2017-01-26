# execute decl command
# --output:start
# test
# my_file.txt
# other.txt
# --output:end

cmd test {
  mkdir dcl_teste
  touch dcl_teste/my_file.txt
  touch dcl_teste/other.txt
  touch dcl_teste/your.cmd

  echo ${args[0]}
  ls dcl_teste | grep ${args[1]}

  rm -rf dcl_teste
}

test txt
