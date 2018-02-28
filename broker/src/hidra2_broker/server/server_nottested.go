//+build !test

package server

import (
	"hidra2_broker/utils"
	"log"
	"net/http"
	"strconv"
)

func StartStatistics() {
	statistics.Writer = new(StatisticInfluxDbWriter)
	statistics.Reset()
	go statistics.Monitor()
}

func Start() {
	StartStatistics()
	mux := utils.NewRouter(listRoutes)
	log.Fatal(http.ListenAndServe("localhost:"+strconv.Itoa(settings.Port), http.HandlerFunc(mux.ServeHTTP)))
}

func ReadConfig(fname string) error {
	return utils.ReadJsonFromFile(fname, &settings)
}
