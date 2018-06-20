package utils

import "net/http"

const (
	// ok codes
	StatusOK = http.StatusOK
)
const (
	//error codes
	StatusError      = http.StatusInternalServerError
	StatusWrongInput = http.StatusBadRequest
	StatusNoData     = http.StatusConflict
)
