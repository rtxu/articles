
env LD_LIBRARY_PATH="./$1" CGO_CFLAGS="-I ./$1" CGO_LDFLAGS="-L ./$1" go run $2

