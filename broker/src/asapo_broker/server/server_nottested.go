//+build !test

package server

import (
	"errors"
	log "asapo_broker/logger"
	"asapo_broker/utils"
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

func ReadConfig(fname string) (log.Level, error) {
	if err := utils.ReadJsonFromFile(fname, &settings); err != nil {
		return log.FatalLevel, err
	}

	if settings.BrokerDbAddress == "" {
		return log.FatalLevel, errors.New("BrokerDbAddress not set")
	}

	if settings.MonitorDbAddress == "" {
		return log.FatalLevel, errors.New("MonitorDbAddress not set")
	}

	if settings.Port == 0 {
		return log.FatalLevel, errors.New("Server port not set")
	}

	if settings.MonitorDbName == "" {
		return log.FatalLevel, errors.New("MonitorDbName not set")
	}

	level, err := log.LevelFromString(settings.LogLevel)

	return level, err
}
