package server

import (
	"net/http"
)

func extractResend(r *http.Request) (string) {
	keys := r.URL.Query()
	resend := keys.Get("resend_nacks")
	delay_ms := keys.Get("delay_ms")
	resend_attempts := keys.Get("resend_attempts")
	resend_params := ""
	if len(resend)!=0 {
		resend_params=delay_ms+"_"+resend_attempts
	}
	return resend_params
}

func routeGetNext(w http.ResponseWriter, r *http.Request) {
	processRequest(w, r, "next", extractResend(r), true)
}
