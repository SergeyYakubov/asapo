package server

import (
	"fmt"
	log "asapo_broker/logger"
	"sync"
	"time"
)

type statisticsWriter interface {
	Write(*serverStatistics) error
}

type serverStatistics struct {
	counter int
	mux     sync.Mutex
	Writer  statisticsWriter
}

func (st *serverStatistics) IncreaseCounter() {
	st.mux.Lock()
	defer st.mux.Unlock()
	st.counter++
}

func (st *serverStatistics) GetCounter() int {
	st.mux.Lock()
	defer st.mux.Unlock()
	return st.counter
}

func (st *serverStatistics) Reset() {
	st.mux.Lock()
	defer st.mux.Unlock()
	st.counter = 0
}

func (st *serverStatistics) WriteStatistic() (err error) {
	defer func() {
		if p := recover(); p != nil {
			err = fmt.Errorf("WriteStatistic error: %v", p)
		}
	}()
	return st.Writer.Write(st)
}

func (st *serverStatistics) Monitor() {
	for {
		time.Sleep(1000 * time.Millisecond)
		logstr := "sending statistics to " + settings.MonitorDbAddress + ", dbname: " + settings.MonitorDbName
		if err := st.WriteStatistic(); err != nil {
			log.Error(logstr + " - " + err.Error())
		} else {
			log.Debug(logstr)
		}
		st.Reset()
	}
}
