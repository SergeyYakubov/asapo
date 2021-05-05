package server

import (
	"io/ioutil"
	"net/http"
)

func routeDeleteStream(w http.ResponseWriter, r *http.Request) {
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		http.Error(w, err.Error(), 500)
		return
	}
	processRequest(w, r, "delete_stream", string(body), false)
}
