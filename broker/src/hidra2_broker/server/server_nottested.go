//+build !test

package server

import (
	"errors"
	log "hidra2_broker/logger"
	"hidra2_broker/utils"
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
	log.Info("Listening on port: " + strconv.Itoa(settings.Port))
	log.Fatal(http.ListenAndServe(":"+strconv.Itoa(settings.Port), http.HandlerFunc(mux.ServeHTTP)))
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
