package server

import (
	"net/http"
)

func routeGetLast(w http.ResponseWriter, r *http.Request) {
	getImage(w, r, "last", 0)
}
