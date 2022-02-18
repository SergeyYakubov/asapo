package server

import (
	pb "asapo_common/generated_proto"
	"errors"
	"github.com/stretchr/testify/assert"
	"testing"
	"time"
)

type mockRpc struct {
	LastSendContainer *pb.BrokerDataPointContainer
}
func (m *mockRpc) Send(container *pb.BrokerDataPointContainer) error {
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
func (m *mockErrorRpc) Send(container *pb.BrokerDataPointContainer) error {
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
	monitoring.BrokerName = "myBroker"

	// Send one file
	monitoring.SendBrokerRequest(
		"consumerId", "pipelineStep", "push", "beamtime", "datasource", "stream",
		123)

	monitoring.sendNow()

	assert.NotNil(t, mockRpc.LastSendContainer)
	assert.Equal(t, mockRpc.LastSendContainer.BrokerName, "myBroker")
	assert.GreaterOrEqual(t, mockRpc.LastSendContainer.TimestampMs, uint64(time.Now().UnixNano() / int64(time.Millisecond)) - 1000)
	assert.LessOrEqual(t, mockRpc.LastSendContainer.TimestampMs, uint64(time.Now().UnixNano() / int64(time.Millisecond)))
	assert.Equal(t, len(mockRpc.LastSendContainer.GroupedBrokerRequests), 1)
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[0].PipelineStepId, "pipelineStep")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[0].ConsumerInstanceId, "consumerId")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[0].Command, "push")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[0].Beamtime, "beamtime")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[0].Source, "datasource")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[0].Stream, "stream")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[0].FileCount, uint64(1))
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[0].TotalFileSize, uint64(123))

	// No files send
	monitoring.sendNow()
	assert.NotNil(t, mockRpc.LastSendContainer)
	assert.Equal(t, mockRpc.LastSendContainer.BrokerName, "myBroker")
	assert.GreaterOrEqual(t, mockRpc.LastSendContainer.TimestampMs, uint64(time.Now().UnixNano() / int64(time.Millisecond)) - 1000)
	assert.LessOrEqual(t, mockRpc.LastSendContainer.TimestampMs, uint64(time.Now().UnixNano() / int64(time.Millisecond)))
	assert.Equal(t, len(mockRpc.LastSendContainer.GroupedBrokerRequests), 0)


	// Send 3 files in 2 groups
	monitoring.SendBrokerRequest(
		"consumerId", "pipelineStep", "push", "beamtime1", "datasource", "stream",
		564)
	monitoring.SendBrokerRequest(
		"consumerId", "pipelineStep", "push", "beamtime1", "datasource", "stream",
		32)
	monitoring.SendBrokerRequest(
		"consumerId", "pipelineStep", "push", "beamtime2", "datasource", "stream",
		401)

	monitoring.sendNow()

	assert.NotNil(t, mockRpc.LastSendContainer)
	assert.Equal(t, mockRpc.LastSendContainer.BrokerName, "myBroker")
	assert.GreaterOrEqual(t, mockRpc.LastSendContainer.TimestampMs, uint64(time.Now().UnixNano() / int64(time.Millisecond)) - 1000)
	assert.LessOrEqual(t, mockRpc.LastSendContainer.TimestampMs, uint64(time.Now().UnixNano() / int64(time.Millisecond)))
	assert.Equal(t, len(mockRpc.LastSendContainer.GroupedBrokerRequests), 2)

	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[0].PipelineStepId, "pipelineStep")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[0].ConsumerInstanceId, "consumerId")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[0].Command, "push")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[0].Beamtime, "beamtime1")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[0].Source, "datasource")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[0].Stream, "stream")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[0].FileCount, uint64(2))
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[0].TotalFileSize, uint64(564+32))

	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[1].PipelineStepId, "pipelineStep")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[0].Command, "push")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[1].ConsumerInstanceId, "consumerId")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[1].Beamtime, "beamtime2")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[1].Source, "datasource")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[1].Stream, "stream")
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[1].FileCount, uint64(1))
	assert.Equal(t, mockRpc.LastSendContainer.GroupedBrokerRequests[1].TotalFileSize, uint64(401))
}

func TestSendNowErrorForwarding(t *testing.T) {
	mockErrorRpc := new(mockErrorRpc)
	monitoring.Sender = mockErrorRpc
	monitoring.BrokerName = "myBroker"

	// Send one file
	monitoring.SendBrokerRequest(
		"consumerId", "pipelineStep", "push", "beamtime", "datasource", "stream",
		123)

	assert.Equal(t, monitoring.sendNow().Error(), "MyMockError from Send")
}
