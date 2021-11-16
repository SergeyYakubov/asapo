package logger

import (
	"errors"
	"strings"
)

type Level uint32

//log levels
const (
	InfoLevel = iota
	DebugLevel
	ErrorLevel
	WarnLevel
	FatalLevel
)

type Logger interface {
	WithFields(args map[string]interface{}) Logger
	Info(args ...interface{})
	Debug(args ...interface{})
	Fatal(args ...interface{})
	Warning(args ...interface{})
	Error(args ...interface{})
	SetLevel(level Level)
	SetSource(source string)
}

var my_logger Logger = &logRusLogger{}

func Info(args ...interface{}) {
	my_logger.Info(args...)
}

func Debug(args ...interface{}) {
	my_logger.Debug(args...)
}

func Warning(args ...interface{}) {
	my_logger.Warning(args...)
}

func Error(args ...interface{}) {
	my_logger.Error(args...)
}

func Fatal(args ...interface{}) {
	my_logger.Fatal(args...)
}

func SetLevel(level Level) {
	my_logger.SetLevel(level)
}

func SetSoucre(source string ){
	my_logger.SetSource(source)
}

func LevelFromString(str string) (Level, error) {
	switch strings.ToLower(str) {
	case "debug":
		return DebugLevel, nil
	case "info":
		return InfoLevel, nil
	case "warning":
		return WarnLevel, nil
	case "error":
		return ErrorLevel, nil
	case "fatal", "none":
		return FatalLevel, nil
	}
	return FatalLevel, errors.New("wrong log level")

}
