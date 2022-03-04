package server

import (
	pb "asapo_common/generated_proto"
	"context"
	"sync"
	"time"
)

type brokerMonitoring struct {
	Sender BrokerMonitoringDataSender
	BrokerName              string

	toBeSendMutex      sync.Mutex
	toBeSendDataPoints []*pb.BrokerRequestDataPoint

	sendingThreadRunningCtx context.Context

	discoveredMonitoringServerUrl 	string
}

func (m *brokerMonitoring) getBrokerToConsumerTransfer(consumerInstanceId string, pipelineStepId string, op string, beamtime string, source string, stream string) *pb.BrokerRequestDataPoint {
	for _, element := range m.toBeSendDataPoints {
		if element.ConsumerInstanceId == consumerInstanceId &&
			element.PipelineStepId == pipelineStepId &&
			element.Command == op &&
			element.Beamtime == beamtime &&
			element.Source == source &&
			element.Stream == stream {
			return element
		}
	}

	newPoint := &pb.BrokerRequestDataPoint{
		PipelineStepId:     pipelineStepId,
		ConsumerInstanceId: consumerInstanceId,
		Command:            op,
		Beamtime:           beamtime,
		Source:             source,
		Stream:             stream,
		FileCount:          0,
		TotalFileSize:      0,
	}

	// Need to insert element
	m.toBeSendDataPoints = append(m.toBeSendDataPoints, newPoint)

	return newPoint
}

func (m *brokerMonitoring) sendNow() error {

	// Lock and quickly exchange the to be sent data
	m.toBeSendMutex.Lock()
	localToBeSendDataPoints := m.toBeSendDataPoints
	m.toBeSendDataPoints = nil
	m.toBeSendMutex.Unlock()


	dataContainer := pb.BrokerDataPointContainer{
		BrokerName:            m.BrokerName,
		TimestampMs:           uint64(time.Now().UnixNano() / int64(time.Millisecond)),
		GroupedBrokerRequests: localToBeSendDataPoints,
	}

	err := m.Sender.Send(&dataContainer)
	if err != nil {
		return err
	}

	return nil
}

func (m *brokerMonitoring) SendBrokerRequest(
	consumerInstanceId string, pipelineStepId string,
	op string, beamtimeId string, datasource string, stream string, size uint64) {

	m.toBeSendMutex.Lock()

	group := m.getBrokerToConsumerTransfer(consumerInstanceId, pipelineStepId, op, beamtimeId, datasource, stream)
	group.FileCount++
	group.TotalFileSize += size

	m.toBeSendMutex.Unlock()
}
