package server

import (
	"net/http"
)

func routeGetSubstreams(w http.ResponseWriter, r *http.Request) {
	keys := r.URL.Query()
	from := keys.Get("from")
	processRequest(w, r, "substreams", from, false)
}
