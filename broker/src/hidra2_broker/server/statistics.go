package server

import (
	"fmt"
	"log"
	"time"
)

type statisticsWriter interface {
	Write(*serverStatistics) error
}

type serverStatistics struct {
	counter int
	Writer  statisticsWriter
}

func (st *serverStatistics) IncreaseCounter() {
	st.counter++
}

func (st *serverStatistics) GetCounter() int {
	return st.counter
}

func (st *serverStatistics) Reset() {
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
		if err := st.WriteStatistic(); err != nil {
			log.Println(err.Error())
		}
		st.Reset()
	}
}
