package server

import (
	"net/http"
)

func routeGetStreams(w http.ResponseWriter, r *http.Request) {
	keys := r.URL.Query()
	from := keys.Get("from")
	processRequest(w, r, "streams", from, false)
}
