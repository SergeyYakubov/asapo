//+build !test

package server

import (
	"log"
        "github.com/influxdata/influxdb1-client/v2"
	"time"
)

type StatisticLogWriter struct {
}

func (writer *StatisticLogWriter) Write(statistics *serverStatistics) error {
	log.Println(statistics.GetCounter())
	return nil
}

type StatisticInfluxDbWriter struct {
}

func (writer *StatisticInfluxDbWriter) Write(statistics *serverStatistics) error {
	c, err := client.NewHTTPClient(client.HTTPConfig{
		Addr: "http://"+ settings.PerformanceDbServer,
	})
	if err != nil {
		return err
	}
	defer c.Close()

	bp, _ := client.NewBatchPoints(client.BatchPointsConfig{
		Database:  settings.PerformanceDbName,
	})

	tags := map[string]string{"Group ID": "0"}
	fields := map[string]interface{}{
		"rate": statistics.GetCounter(),
	}
	pt, err := client.NewPoint("RequestsRate", tags, fields, time.Now())
	if err != nil {
		return err
	}
	bp.AddPoint(pt)

	return c.Write(bp)
}
