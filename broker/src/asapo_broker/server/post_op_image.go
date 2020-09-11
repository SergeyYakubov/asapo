package server

import (
	"encoding/json"
	"io/ioutil"
	"net/http"
	"strconv"
)

type ImageOp struct {
	Id int
	Op string
	Params map[string]interface{} `json:",omitempty"`
}
func routeImageOp(w http.ResponseWriter, r *http.Request) {
	body, err := ioutil.ReadAll(r.Body)
	if err != nil {
		http.Error(w, err.Error(), 500)
		return
	}

	id_str, ok := extractRequestParametersID(r)
	if !ok {
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	id, err := strconv.Atoi(id_str)
	if err != nil {
		w.WriteHeader(http.StatusBadRequest)
		return
	}

	var  op ImageOp
	err = json.Unmarshal(body, &op)
	if err != nil {
		http.Error(w, err.Error(), http.StatusBadRequest)
		return
	}
	op.Id = id
	bOp,_ := json.Marshal(&op)
	processRequest(w, r, op.Op, string(bOp), true)
}
