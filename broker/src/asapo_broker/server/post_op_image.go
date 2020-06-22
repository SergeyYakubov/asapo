package server

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
)

type ImageOp struct {
	Op string
}
func routeImageOp(w http.ResponseWriter, r *http.Request) {
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		http.Error(w, err.Error(), 500)
		return
	}

	id, ok := extractRequestParametersID(r)
	if !ok {
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	var  op ImageOp
	err = json.Unmarshal(body, &op)
	if err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}

	fmt.Println(op)

	processRequest(w, r, "ackimage", id, true)
}
