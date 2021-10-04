package server

import (
	"asapo_common/utils"
	"net/http"
)

func routeGetStreams(w http.ResponseWriter, r *http.Request) {
	keys := r.URL.Query()
	from := keys.Get("from")
	filter := keys.Get("filter")
	utils.EncodeTwoStrings(from,filter)
	encoded := utils.EncodeTwoStrings(from,filter)
	processRequest(w, r, "streams", encoded, false)
}
