package server

import (
	"fmt"
	"net/http"
)

func routeGetStreams(w http.ResponseWriter, r *http.Request) {
	fmt.Println(r.RequestURI)
	keys := r.URL.Query()
	from := keys.Get("from")
	filter := keys.Get("filter")
	processRequest(w, r, "streams", from+"_"+filter, false)
}
