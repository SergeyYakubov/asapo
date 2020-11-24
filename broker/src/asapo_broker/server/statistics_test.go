package server

import (
	"errors"
	"github.com/stretchr/testify/assert"
	"github.com/stretchr/testify/mock"
	"testing"
)

type mockWriter struct {
	mock.Mock
}

func (writer *mockWriter) Init() error {
	args := writer.Called()
	return args.Error(0)
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

	mock_writer.On("Init").Return(nil)
	mock_writer.On("Write", &statistics).Return(nil)

	statistics.Init()
	statistics.Reset()
	statistics.IncreaseCounter()
    counter := statistics.GetCounter()

	err := statistics.WriteStatistic()
	assert.Nil(t, err, "Statistics written")
	assert.Equal(t, 1, counter, "counter")

	assertMockWriterExpectations(t, mock_writer)
}

func TestInitError(t *testing.T) {
	mock_writer := new(mockWriter)
	statistics.Writer = mock_writer

	mock_writer.On("Init").Return(errors.New("error"))

	statistics.Init()

	err := statistics.WriteStatistic()
	assert.NotNil(t, err, "Statistics init error")

	assertMockWriterExpectations(t, mock_writer)
}


func TestWriteStatisticsCatchesError(t *testing.T) {
	statistics.Writer = nil

	err := statistics.WriteStatistic()

	assert.NotNil(t, err, "Error with nil pointer")
}
