package request_handler

type Agent interface {
	GetReceivers() ([]byte, error)
	Init(int,[]string) error
}

