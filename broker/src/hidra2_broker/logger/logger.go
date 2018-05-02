package logger

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
	Info(args ...interface{})
	Debug(args ...interface{})
	Fatal(args ...interface{})
	Warning(args ...interface{})
	Error(args ...interface{})
	SetLevel(level string)
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

func SetLevel(level string) {
	my_logger.SetLevel(level)
}
