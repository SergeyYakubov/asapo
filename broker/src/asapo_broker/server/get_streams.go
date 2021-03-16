package server

import (
	"net/http"
)

func routeGetStreams(w http.ResponseWriter, r *http.Request) {
	keys := r.URL.Query()
	from := keys.Get("from")
	filter := keys.Get("filter")
	processRequest(w, r, "streams", from+"_"+filter, false)
}
