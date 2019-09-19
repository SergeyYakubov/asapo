package server

import (
	log "asapo_common/logger"
	"fmt"
	"sync"
	"time"
)

type statisticsWriter interface {
	Write(*serverStatistics) error
	Init() error
}

func (st *serverStatistics) Init() {
	st.mux.Lock()
	defer st.mux.Unlock()
	if err := st.Writer.Init(); err != nil {
		log.Warning("cannot initialize statistic writer: " + err.Error())
	} else {
		log.Debug("initialized statistic at " + settings.PerformanceDbServer + " for " + settings.PerformanceDbName)
	}
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
		logstr := "sending statistics to " + settings.PerformanceDbServer + ", dbname: " + settings.PerformanceDbName
		if err := st.WriteStatistic(); err != nil {
			log.Error(logstr + " - " + err.Error())
		} else {
			log.Debug(logstr)
		}
		st.Reset()
	}
}
