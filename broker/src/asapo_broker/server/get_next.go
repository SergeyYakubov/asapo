package server

import (
	"net/http"
)

func routeGetNext(w http.ResponseWriter, r *http.Request) {
	getImage(w, r, "next", 0)
}
