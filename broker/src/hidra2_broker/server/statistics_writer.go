//+build !test

package server

import (
	"github.com/influxdata/influxdb/client/v2"
	"log"
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
		Addr: "http://localhost:8086",
	})
	if err != nil {
		return err
	}
	defer c.Close()

	bp, _ := client.NewBatchPoints(client.BatchPointsConfig{
		Database:  "db",
		Precision: "s",
	})

	//	tags := map[string]string{"rate": "rate-total"}
	fields := map[string]interface{}{
		"rate": statistics.GetCounter(),
	}
	pt, err := client.NewPoint("RequestsRate", nil, fields, time.Now())
	if err != nil {
		return err
	}
	bp.AddPoint(pt)

	return c.Write(bp)
}
