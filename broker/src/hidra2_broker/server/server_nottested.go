//+build !test

package server

import (
	"hidra2_broker/utils"
	"log"
	"net/http"
	"strconv"
	"errors"
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
	if err := utils.ReadJsonFromFile(fname, &settings); err != nil {
		return err
	}

	if settings.BrokerDbAddress == "" {
		return errors.New("BrokerDbAddress not set")
	}

	if settings.MonitorDbAddress == "" {
		return errors.New("MonitorDbAddress not set")
	}

	if settings.Port == 0 {
		return errors.New("Server port not set")
	}

	if settings.MonitorDbName == "" {
		return errors.New("MonitorDbName not set")
	}

	return nil
}
