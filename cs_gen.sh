find src test \
    -name '*.h' \
    -o -name '*.cpp' \
    > cscope.files

cscope -b -q -R
