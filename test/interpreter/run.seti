# Copyright 2016 Alex Silva Torres
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Run tests for shpp interpreter

func get_file_lines(file) {
    content = $(cat ${file})

    lines = []

    for line in content {
        if line[0] == "#" {
          lines.append(line[1:])
        }
    }

    return lines
}

func get_file_output(file) {

    lines = get_file_lines(file)

    get = false
    out_lines = []

    for line in lines {
        if line.trim() == "--output:end" {
            get = false
        }

        if get {
            out_lines.append(line.trim())
        }

        if line.trim() == "--output:start" {
            get = true
        }
    }

    return out_lines
}

func compare_lines(out_lines, expected_lines) {
    for out_line, expected_line in out_lines, expected_lines {
        if expected_line[0] == "*" {
            if out_line.find("Error") == false {
                return false
            }

            continue
        }
        if out_line != expected_line {
            return false
        }
    }

    return true
}

# shpp interpreter has several limitations at this moment
# on the future this test must be improved

shpp_path = "../build/shell/shpp"
test_path = "../test/interpreter/"

folders = []

for p in $(ls ${test_path}) {
    ptest = test_path + p

    if path.is_dir(ptest) {
        folders.append(ptest)
    }
}

for folder in folders {
    for f in $(ls ${folder}) {
        file = folder +"/"+ f
        fres = get_file_output(file)
        fout = $(./${shpp_path} ${file})

        if compare_lines(fout, fres) {
            echo test ${file} correct
        } else {
            echo test ${file} error
            print("output: ", string(fout))
            print("expected: ", fres.join("\n"))
        }
    }
}


