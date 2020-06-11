package server

import (
	"net/http"
)

func extractLimits(r *http.Request) (string,string, bool) {
	keys := r.URL.Query()
	from := keys.Get("from")
	to := keys.Get("to")
	if len(from)==0 {
		from="0"
	}
	if len(to)==0 {
		to="0"
	}
	return from,to, true
}


func routeGetNacks(w http.ResponseWriter, r *http.Request) {
	from,to, ok := extractLimits(r)
	if !ok {
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	processRequest(w, r, "nacks", from+"_"+to, true)
}
