//+build !release

package logger

import (
	"github.com/stretchr/testify/mock"
)

type MockLogger struct {
	mock.Mock
}

var MockLog MockLogger

func SetMockLog() {
	my_logger = &MockLog
}

func (l *MockLogger) WithFields(args map[string]interface{}) Logger {
	l.Called(args)
	return l
}

func UnsetMockLog() {
	my_logger = &logRusLogger{}
}

func (l *MockLogger) SetSource(source string) {
	l.Called(source)
	return
}

func (l *MockLogger) Info(args ...interface{}) {
	l.Called(args...)
	return
}

func (l *MockLogger) Debug(args ...interface{}) {
	l.Called(args...)
	return
}

func (l *MockLogger) Error(args ...interface{}) {
	l.Called(args...)
	return
}

func (l *MockLogger) Warning(args ...interface{}) {
	l.Called(args...)
	return
}

func (l *MockLogger) Fatal(args ...interface{}) {
	l.Called(args...)
	return
}

func (l *MockLogger) SetLevel(level Level) {
	l.Called(level)
	return
}
