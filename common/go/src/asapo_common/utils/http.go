package utils

import (
	"encoding/json"
	"net/http"
)

func ExtractRequest(r *http.Request, request interface{}) error {
	decoder := json.NewDecoder(r.Body)
	return decoder.Decode(request)
}

func WriteServerError(w http.ResponseWriter, err error,code int) {
	w.WriteHeader(code)
	w.Write([]byte(err.Error()))
}