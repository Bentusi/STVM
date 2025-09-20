#!/usr/bin/bash

# 将本目录下所有st文件调用上层目录的st_compiler执行
# 并判断程序返回值是否正确

for file in *.st; do
    echo "【编译并运行 $file ...】"
    ../st_compiler >/dev/null 2>&1 "$file"
    if [ $? -ne 0 ]; then
        echo "【运行 $file 失败！】"
        exit 1
    fi
    echo "【$file 运行成功！】"
    echo "-----------------------"
done