//+build !test

package server

import (
	log "asapo_common/logger"
	"time"
)

func (st *serverStatistics) Monitor() {
	for {
		time.Sleep(1000 * time.Millisecond)
		if err := st.WriteStatistic(); err != nil {
		    logstr := "sending statistics to " + settings.PerformanceDbServer + ", dbname: " + settings.PerformanceDbName
			log.Error(logstr + " - " + err.Error())
		}
		st.Reset()
	}
}
