package server

import (
	pb "asapo_common/generated_proto"
	"context"
	"sync"
	"time"
)

type brokerMonitoring struct {
	Sender BrokerMonitoringDataSender
	FtsName string

	toBeSendMutex      sync.Mutex
	toBeSendDataPoints []*pb.FdsToConsumerDataPoint

	sendingThreadRunningCtx context.Context

	discoveredMonitoringServerUrl 	string
}

func (m *brokerMonitoring) getFtsToConsumerTransfer(consumerInstanceId string, pipelineStepId string,
	beamtime string, source string, stream string) *pb.FdsToConsumerDataPoint {
	for _, element := range m.toBeSendDataPoints {
		if element.ConsumerInstanceId == consumerInstanceId &&
			element.PipelineStepId == pipelineStepId &&
			element.Beamtime == beamtime &&
			element.Source == source &&
			element.Stream == stream {
			return element
		}
	}

	newPoint := &pb.FdsToConsumerDataPoint{
		PipelineStepId:     pipelineStepId,
		ConsumerInstanceId: consumerInstanceId,
		Beamtime:           beamtime,
		Source:             source,
		Stream:             stream,
		FileCount:          0,
		TotalFileSize:      0,
		TotalTransferSendTimeInMicroseconds: 0,
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


	dataContainer := pb.FtsToConsumerDataPointContainer{
		FtsName:           	   m.FtsName,
		TimestampMs:           uint64(time.Now().UnixNano() / int64(time.Millisecond)),
		GroupedFdsTransfers:   localToBeSendDataPoints,
	}

	err := m.Sender.Send(&dataContainer)
	if err != nil {
		return err
	}

	return nil
}

func (m *brokerMonitoring) SendFileTransferServiceRequest(
	consumerInstanceId string, pipelineStepId string, beamtimeId string, datasource string, stream string,
	transferTimeMicroseconds uint64, size uint64) {

	m.toBeSendMutex.Lock()

	group := m.getFtsToConsumerTransfer(consumerInstanceId, pipelineStepId, beamtimeId, datasource, stream)
	group.FileCount++
	group.TotalFileSize += size
	group.TotalTransferSendTimeInMicroseconds += transferTimeMicroseconds

	m.toBeSendMutex.Unlock()
}
