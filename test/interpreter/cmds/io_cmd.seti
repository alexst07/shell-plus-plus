# execute simple cmds
# --output:start
# alex
# xela
# xael
# --output:end

path_test = "iotest__3"
mkdir ${path_test}

ls ${path_test} | grep t1 > ftest__3
cat < ftest__3

my_var = "alex\nxela\nxael\nlexa"

cat << ${my_var} | grep alex > ${path_test}/falex__3
cat << ${my_var} | grep xela >> ${path_test}/falex__3
cat <<< my_var | grep xael >> ${path_test}/falex__3
cat ${path_test}/falex__3

rm -rf ${path_test}

ls | grep ${path_test}
