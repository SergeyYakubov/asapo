package utils

import (
	"errors"
	"github.com/gorilla/mux"
	"net/http"
	"strconv"
	"strings"
)


func VersionToNumber(ver string) int {
	ver = strings.TrimPrefix(ver,"v")
	floatNum, err := strconv.ParseFloat(ver, 64)
	if err!=nil {
		return 0
	}
	return int(floatNum*1000)
}


func ExtractVersion(r *http.Request) (int, error) {
	vars := mux.Vars(r)
	ver_str, ok := vars["apiver"]
	if !ok {
		return 0, errors.New("cannot extract version")
	}
	ver := VersionToNumber(ver_str)
	if ver == 0 {
		return 0, errors.New("cannot extract version")
	}
	return ver, nil
}

func PrecheckApiVersion(w http.ResponseWriter, r *http.Request, currentVersion string) (apiVer int, ok bool) {
	apiVer, err := ExtractVersion(r)
	if err != nil {
		WriteServerError(w, err, http.StatusBadRequest)
		return 0, false
	}
	if apiVer > VersionToNumber(currentVersion) {
		WriteServerError(w, errors.New("version not supported"), http.StatusUnsupportedMediaType)
		return 0, false
	}
	return apiVer, true
}
