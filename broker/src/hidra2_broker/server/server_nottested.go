//+build !test

package server

import (
	"hidra2_broker/utils"
	"log"
	"net/http"
	"fmt"
	"time"
	"sync"
)

func Start() {
//	test(os.Args[1])
//	return
	mux := utils.NewRouter(listRoutes)
	log.Fatal(http.ListenAndServe("127.0.0.1:5005", http.HandlerFunc(mux.ServeHTTP)))
}

func test(db_name string) {
	nattempts := 50000
	nconns := 8
	start := time.Now()
	nattempts = nattempts / nconns
	counts := make([]int, nconns)
	nbad := make([]int, nconns)
	ngood := make([]int, nconns)

	var waitGroup sync.WaitGroup
	waitGroup.Add(nconns)

	a := func(i int) {
		for ; counts[i] < nattempts; counts[i]++ {
			_,code := getNextRecord(db_name)
			if code != utils.StatusOK {
				nbad[i]++
			} else {
				ngood[i]++
			}
		}
		waitGroup.Done()
	}

	for i := 0; i < nconns; i++ {
		go a(i)
	}
	waitGroup.Wait()
	sum := 0
	for i := range counts {
		sum += counts[i]
	}
	elapsed := time.Since(start)
	fmt.Println("rate:", float64(sum)/elapsed.Seconds())
	fmt.Println("good,bad:", ngood, nbad)
}
