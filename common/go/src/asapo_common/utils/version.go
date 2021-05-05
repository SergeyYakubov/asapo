package utils

import (
	"errors"
	"github.com/gorilla/mux"
	"net/http"
	"strconv"
	"strings"
)


type VersionNum struct{
	Major int
	Minor int
	Id int
}

func ParseVersion(ver string) (result VersionNum,err error ) {
	ver = strings.TrimPrefix(ver,"v")
	vers := strings.Split(ver,".")
	if len(vers)!=2 {
		err = errors.New("cannot parse version")
		return
	}
	maj, err := strconv.Atoi(vers[0])
	if err!=nil {
		err = errors.New("cannot parse version")
		return
	}
	min, err := strconv.Atoi(vers[1])
	if err!=nil {
		err = errors.New("cannot parse version")
		return
	}
	result.Major = maj
	result.Minor = min
	result.Id = maj*1000+min
	return
}


func ExtractVersion(r *http.Request) (VersionNum,error ) {
	vars := mux.Vars(r)
	ver_str, ok := vars["apiver"]
	if !ok {
		return VersionNum{},errors.New("cannot extract version")
	}
	return ParseVersion(ver_str)
}

func PrecheckApiVersion(w http.ResponseWriter, r *http.Request, currentVersion string) (VersionNum, bool) {
	ver, err := ExtractVersion(r)
	if err != nil {
		WriteServerError(w, err, http.StatusBadRequest)
		return VersionNum{}, false
	}
	curVer,_ := ParseVersion(currentVersion)
	if ver.Id > curVer.Id {
		WriteServerError(w, errors.New("version not supported"), http.StatusUnsupportedMediaType)
		return VersionNum{}, false
	}
	return ver, true
}
