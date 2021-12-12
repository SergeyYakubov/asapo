package server

import (
	"net/http"
)

func routeGetGroupedLast(w http.ResponseWriter, r *http.Request) {
	processRequest(w, r, "groupedlast", "", true)
}
