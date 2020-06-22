package server

import (
	"net/http"
)

func routeGetLastAck(w http.ResponseWriter, r *http.Request) {
	processRequest(w, r, "lastack", "", true)
}
