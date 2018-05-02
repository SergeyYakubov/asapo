package logger

import (
	log "github.com/sirupsen/logrus"
	"strings"
)

type logRusLogger struct {
	logger_entry *log.Entry
}

func (l *logRusLogger) entry() *log.Entry {
	if l.logger_entry != nil {
		return l.logger_entry
	}

	formatter := &log.JSONFormatter{
		FieldMap: log.FieldMap{
			log.FieldKeyMsg: "message",
		},
		TimestampFormat: "2006-01-02 15:04:05.000",
	}

	log.SetFormatter(formatter)

	l.logger_entry = log.WithFields(log.Fields{
		"source": "broker",
	})

	return l.logger_entry

}

func (l *logRusLogger) Info(args ...interface{}) {
	l.entry().Info(args...)
	return
}

func (l *logRusLogger) Debug(args ...interface{}) {
	l.entry().Debug(args...)
	return
}

func (l *logRusLogger) Error(args ...interface{}) {
	l.entry().Error(args...)
	return
}

func (l *logRusLogger) Warning(args ...interface{}) {
	l.entry().Warning(args...)
	return
}

func (l *logRusLogger) Fatal(args ...interface{}) {
	l.entry().Fatal(args...)
	return
}

func (l *logRusLogger) SetLevel(level string) {
	logrus_level := log.InfoLevel
	switch strings.ToLower(level) {
	case "debug":
		logrus_level = log.DebugLevel
	case "info":
		logrus_level = log.InfoLevel
	case "warning":
		logrus_level = log.WarnLevel
	case "error":
		logrus_level = log.ErrorLevel
	case "fatal":
		logrus_level = log.FatalLevel
	}

	log.SetLevel(logrus_level)
	return
}
