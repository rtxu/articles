package main

/*
#include "segfault.h"

#cgo LDFLAGS: -lsegfault
*/
import "C"
import (
    "fmt"
    "time"
)

func main() {
    time.AfterFunc(3*time.Second, func() { C.segfault_in_another_thread() })

    for now := range time.Tick(1 * time.Second) {
        fmt.Println("in main: ", now)
    }
}
