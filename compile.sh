clang -o build/client \
src/client/*.c \
-ggdb3 \
--std=c11 \
-Wall \
-I./include \
`pkg-config --cflags --libs gtk+-3.0`

clang -o build/server src/server/*.c -Iinclude