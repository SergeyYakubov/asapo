//+build !test

package server

import (
	"hidra2_broker/utils"
	"log"
	"net/http"
)

func Start() {
	mux := utils.NewRouter(listRoutes)
	log.Fatal(http.ListenAndServe("127.0.0.1:5005", http.HandlerFunc(mux.ServeHTTP)))
}
