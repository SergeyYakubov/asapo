package utils

import "net/http"

const (
	// ok codes
	StatusOK = http.StatusOK
)
const (
	//error codes
	StatusTransactionInterrupted = http.StatusInternalServerError
	StatusServiceUnavailable     = http.StatusServiceUnavailable
	StatusWrongInput             = http.StatusBadRequest
	StatusNoData                 = http.StatusConflict
	StatusPartialData            = http.StatusPartialContent
)
