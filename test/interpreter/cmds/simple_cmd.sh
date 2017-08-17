# execute simple cmds
# --output:start
# t1
# t2
# t3
# t1
# t2
# --output:end

path_test = "test__2"

if path(path_test).exists() {
  rm -r ${path_test}
}

mkdir ${path_test}

touch ${path_test}/t1
touch ${path_test}/t2
touch ${path_test}/t3

ls ${path_test}
ls ${path_test} | grep t1
ls ${path_test} | grep t2

for f in $(ls ${path_test}) {
  rm ${path_test + "/" + f}
}

ls ${path_test}
rm -r ${path_test}

ls | grep ${path_test}
