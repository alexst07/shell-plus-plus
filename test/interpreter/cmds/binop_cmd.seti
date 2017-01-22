# execute simple cmds
# --output:start
# t1
# t2
# ---
# t1
# ---
# t2
# t3
# ---
# t2
# ---
# --output:end

path_test = "test__3"
mkdir ${path_test}

touch ${path_test}/t1
touch ${path_test}/t2
touch ${path_test}/t3

ls ${path_test} | grep t1 && ls ${path_test} | grep t2
echo ---
ls ${path_test} | grep t1 || ls ${path_test} | grep t2
echo ---
ls ${path_test} | grep t2 && ls ${path_test} | grep t3
echo ---
ls ${path_test} | grep t2 || ls ${path_test} | grep t3
echo ---

for f in $(ls ${path_test}) {
  rm ${path_test + "/" + f}
}

ls ${path_test}
rm -rf ${path_test}

ls | grep ${path_test}
