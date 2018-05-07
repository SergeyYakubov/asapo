package server

import (
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"testing"
)

type mockWriter struct {
	mock.Mock
}

func (writer *mockWriter) Write(statistics *serverStatistics) error {
	args := writer.Called(statistics)
	return args.Error(0)
}

func assertMockWriterExpectations(t *testing.T, mock_writer *mockWriter) {
	mock_writer.AssertExpectations(t)
	mock_writer.ExpectedCalls = nil
}

func TestWriteStatisticsOK(t *testing.T) {
	mock_writer := new(mockWriter)
	statistics.Writer = mock_writer
	statistics.Reset()
	statistics.IncreaseCounter()

	mock_writer.On("Write", &statistics).Return(nil)

	err := statistics.WriteStatistic()
	assert.Nil(t, err, "Statistics written")

	assertMockWriterExpectations(t, mock_writer)
}

func TestWriteStatisticsCatchesError(t *testing.T) {
	statistics.Writer = nil

	err := statistics.WriteStatistic()

	assert.NotNil(t, err, "Error with nil pointer")
}
