package server

import (
	pb "asapo_common/generated_proto"
	log "asapo_common/logger"
	"context"
	"google.golang.org/grpc"
	"os"
	"strconv"
	"time"
)

type BrokerMonitoringDataSender interface {
	Init(serverUrl string) error
	Send(container *pb.FtsToConsumerDataPointContainer) error
	IsInitialized() bool
}

type gRPCBrokerMonitoringDataSender struct {
	client pb.AsapoMonitoringIngestServiceClient
}

func (g *gRPCBrokerMonitoringDataSender) IsInitialized() bool {
	return g.client != nil
}

func (g *gRPCBrokerMonitoringDataSender) Init(serverUrl string) error {
	var opts []grpc.DialOption
	opts = append(opts, grpc.WithInsecure())
	conn, err := grpc.Dial(serverUrl, opts...)
	if err != nil {
		return err
	}

	g.client = pb.NewAsapoMonitoringIngestServiceClient(conn)

	return nil
}

func (g *gRPCBrokerMonitoringDataSender) Send(container *pb.FtsToConsumerDataPointContainer) error {
	_, err := g.client.InsertFtsDataPoints(context.TODO(), container)

	if err != nil {
		return err
	}

	return nil
}

func (m *brokerMonitoring) reinitializeSender() error {
	if settings.MonitoringServerUrl == "auto" {
		fetchedMonitoringServerUrl, err := discoveryService.GetMonitoringServerUrl()
		if err != nil {
			return err
		}
		m.discoveredMonitoringServerUrl = fetchedMonitoringServerUrl
	} else {
		m.discoveredMonitoringServerUrl = settings.MonitoringServerUrl
	}

	err := m.Sender.Init(m.discoveredMonitoringServerUrl)
	if err != nil {
		return err
	}

	return nil
}

func (m *brokerMonitoring) Init() error {
	hostname, err := os.Hostname()
	if err != nil {
		hostname = "hostnameerror"
	}
	m.FtsName = "fts_" + hostname + "_" + strconv.Itoa(os.Getpid())

	return nil
}

func (m *brokerMonitoring) RunThread() {
	time.Sleep(5000 * time.Millisecond)
	globalStart := time.Now()

	sendingInterval := 5000 * time.Millisecond
	time.Sleep(sendingInterval)

	waitForNextIteration := func(iterationStartTime time.Time) {
		timeTook := time.Since(iterationStartTime)
		if timeTook < sendingInterval {
			sleepTime := sendingInterval - (time.Since(globalStart) % sendingInterval)
			time.Sleep(sleepTime)
		}
	}

	for {
		iterationStart := time.Now()
		if !m.Sender.IsInitialized() {
			err := m.reinitializeSender()
			if err != nil {
				log.Warning("monitoring.reinitializeSender failed " , err.Error())
				waitForNextIteration(iterationStart)
				continue
			}
		}

		err := m.sendNow()

		if err != nil {
			logString := "sending monitoring data to '" + m.discoveredMonitoringServerUrl + "'"
			log.Error(logString + " - " + err.Error())

			err = m.reinitializeSender()
			if err != nil {
				log.Error("reinitializeSender also failed - " + err.Error())
			}
		}

		waitForNextIteration(iterationStart)
	}
}
