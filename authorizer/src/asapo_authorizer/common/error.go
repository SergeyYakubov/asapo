package common

type ServerError struct {
	Code int
	Message    string
}

func (e *ServerError) Error() string {
	return e.Message
}
