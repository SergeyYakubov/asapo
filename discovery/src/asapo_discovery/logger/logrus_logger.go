package logger

import (
	log "github.com/sirupsen/logrus"
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
		"source": "discovery",
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

func (l *logRusLogger) SetLevel(level Level) {
	logrusLevel := log.InfoLevel
	switch level {
	case DebugLevel:
		logrusLevel = log.DebugLevel
	case InfoLevel:
		logrusLevel = log.InfoLevel
	case WarnLevel:
		logrusLevel = log.WarnLevel
	case ErrorLevel:
		logrusLevel = log.ErrorLevel
	case FatalLevel:
		logrusLevel = log.FatalLevel
	}

	log.SetLevel(logrusLevel)
	return
}
