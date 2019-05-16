# glob test
# --output:start
# ./b.txt ./c.txt ./a.txt
# *.txt
# *.err
# ./adir ./testdir ./cdir ./bdir
# ./bdir
# *dir
# --output:end

files = ["a.txt", "b.txt", "c.txt", "file"]
dirs = ["adir", "bdir", "cdir", "testdir"]

rm -rf test
mkdir test
cd test

for f in files {
  touch ${f}
}

for d in dirs {
  mkdir ${d}
}

echo *.txt
echo "*.txt"
echo *.err
echo *dir
echo bdir
echo "*dir"

for f in files {
  rm ${f}
}

for d in dirs {
  rm -rf ${d}
}

cd ..
rm -rf test
