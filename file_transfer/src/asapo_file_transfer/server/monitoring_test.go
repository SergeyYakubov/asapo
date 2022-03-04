package server

import (
	pb "asapo_common/generated_proto"
	"errors"
	"github.com/stretchr/testify/assert"
	"testing"
	"time"
)

type mockRpc struct {
	LastSendContainer *pb.FtsToConsumerDataPointContainer
}
func (m *mockRpc) Send(container *pb.FtsToConsumerDataPointContainer) error {
	m.LastSendContainer = container
	return nil
}
func (m *mockRpc) Init(serverUrl string) error {
	return nil
}
func (m *mockRpc) IsInitialized() bool {
	return true
}

type mockErrorRpc struct {
}
func (m *mockErrorRpc) Send(container *pb.FtsToConsumerDataPointContainer) error {
	return errors.New("MyMockError from Send")
}
func (m *mockErrorRpc) Init(serverUrl string) error {
	return errors.New("MyMockError from Init")
}
func (m *mockErrorRpc) IsInitialized() bool {
	return true
}

func TestMonitoringCycle(t *testing.T) {
	mockRpc := new(mockRpc)
	monitoring.Sender = mockRpc
	monitoring.FtsName = "myFst"

	// Send one file
	monitoring.SendFileTransferServiceRequest(
		"consumerId", "pipelineStep", "beamtime", "datasource", "stream",
		123, 10000)

	monitoring.sendNow()

	assert.NotNil(t, mockRpc.LastSendContainer)
	assert.Equal(t, mockRpc.LastSendContainer.FtsName, "myFst")
	assert.GreaterOrEqual(t, mockRpc.LastSendContainer.TimestampMs, uint64(time.Now().UnixNano() / int64(time.Millisecond)) - 1000)
	assert.LessOrEqual(t, mockRpc.LastSendContainer.TimestampMs, uint64(time.Now().UnixNano() / int64(time.Millisecond)))
	assert.Equal(t, len(mockRpc.LastSendContainer.GroupedFdsTransfers), 1)
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[0].PipelineStepId, "pipelineStep")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[0].ConsumerInstanceId, "consumerId")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[0].Beamtime, "beamtime")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[0].Source, "datasource")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[0].Stream, "stream")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[0].FileCount, uint64(1))
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[0].TotalTransferSendTimeInMicroseconds, uint64(123))
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[0].TotalFileSize, uint64(10000))

	// No files send
	monitoring.sendNow()
	assert.NotNil(t, mockRpc.LastSendContainer)
	assert.Equal(t, mockRpc.LastSendContainer.FtsName, "myFst")
	assert.GreaterOrEqual(t, mockRpc.LastSendContainer.TimestampMs, uint64(time.Now().UnixNano() / int64(time.Millisecond)) - 1000)
	assert.LessOrEqual(t, mockRpc.LastSendContainer.TimestampMs, uint64(time.Now().UnixNano() / int64(time.Millisecond)))
	assert.Equal(t, len(mockRpc.LastSendContainer.GroupedFdsTransfers), 0)


	// Send 3 files in 2 groups
	monitoring.SendFileTransferServiceRequest(
		"consumerId", "pipelineStep", "beamtime1", "datasource", "stream",
		564, 567123)
	monitoring.SendFileTransferServiceRequest(
		"consumerId", "pipelineStep", "beamtime1", "datasource", "stream",
		32, 8912)
	monitoring.SendFileTransferServiceRequest(
		"consumerId", "pipelineStep", "beamtime2", "datasource", "stream",
		401, 9874)

	monitoring.sendNow()

	assert.NotNil(t, mockRpc.LastSendContainer)
	assert.Equal(t, mockRpc.LastSendContainer.FtsName, "myFst")
	assert.GreaterOrEqual(t, mockRpc.LastSendContainer.TimestampMs, uint64(time.Now().UnixNano() / int64(time.Millisecond)) - 1000)
	assert.LessOrEqual(t, mockRpc.LastSendContainer.TimestampMs, uint64(time.Now().UnixNano() / int64(time.Millisecond)))
	assert.Equal(t, len(mockRpc.LastSendContainer.GroupedFdsTransfers), 2)

	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[0].PipelineStepId, "pipelineStep")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[0].ConsumerInstanceId, "consumerId")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[0].Beamtime, "beamtime1")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[0].Source, "datasource")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[0].Stream, "stream")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[0].FileCount, uint64(2))
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[0].TotalTransferSendTimeInMicroseconds, uint64(564+32))
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[0].TotalFileSize, uint64(567123+8912))

	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[1].PipelineStepId, "pipelineStep")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[1].ConsumerInstanceId, "consumerId")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[1].Beamtime, "beamtime2")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[1].Source, "datasource")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[1].Stream, "stream")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[1].FileCount, uint64(1))
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[1].TotalTransferSendTimeInMicroseconds, uint64(401))
	assert.Equal(t, mockRpc.LastSendContainer.GroupedFdsTransfers[1].TotalFileSize, uint64(9874))
}

func TestSendNowErrorForwarding(t *testing.T) {
	mockErrorRpc := new(mockErrorRpc)
	monitoring.Sender = mockErrorRpc
	monitoring.FtsName = "myFst"

	// Send one file
	monitoring.SendFileTransferServiceRequest(
		"consumerId", "pipelineStep", "beamtime", "datasource", "stream",
		123, 10000)

	assert.Equal(t, monitoring.sendNow().Error(), "MyMockError from Send")
}
