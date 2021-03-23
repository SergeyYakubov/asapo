package server

import (
	"asapo_common/logger"
	"asapo_common/version"
	"asapo_discovery/common"
	"asapo_discovery/protocols"
	"encoding/json"
	"errors"
	"github.com/gorilla/mux"
	"net/http"
)

type versionInfo struct {
	CoreServices               string
	ClientConsumerProtocol     string
	ClientProducerProtocol     string
	ClientSupported            string
	SupportedProducerProtocols []string
	SupportedConsumerProtocols []string
}

func extractProtocol(r *http.Request) (string, error) {
	keys := r.URL.Query()
	protocol := keys.Get("protocol")
	if protocol=="" {
		return "", errors.New("cannot extract protocol from request")
	}
	return protocol, nil
}


func extractVersion(r *http.Request) (int, error) {
	vars := mux.Vars(r)
	ver_str, ok := vars["apiver"]
	if !ok {
		return 0, errors.New("cannot extract version")
	}

	ver := common.VersionToNumber(ver_str)
	if ver == 0 {
		return 0, errors.New("cannot extract version")
	}
	return ver, nil
}

func routeGetVersion(w http.ResponseWriter, r *http.Request) {
	log_str := "processing get version"
	logger.Debug(log_str)

	if ok := checkDiscoveryApiVersion(w, r);!ok{
		return
	}
	keys := r.URL.Query()
	client := keys.Get("client")
	protocol := keys.Get("protocol")
	info, err := getVersionInfo(client, protocol)
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(err.Error()))
		logger.Error (log_str+" - "+err.Error())
		return
	}
	resp, _ := json.Marshal(&info)
	w.Write(resp)
}

func checkDiscoveryApiVersion(w http.ResponseWriter, r *http.Request) bool {
	apiVer, err := extractVersion(r)
	if err != nil {
		w.WriteHeader(http.StatusBadRequest)
		w.Write([]byte(err.Error()))
		return false
	}
	if apiVer > common.VersionToNumber(common.ApiVersion) {
		w.WriteHeader(http.StatusUnsupportedMediaType)
		w.Write([]byte("version not supported"))
		return false
	}
	return true
}

func getVersionInfo(client string, ver string) (versionInfo, error) {
	info, err := getCoreInfo()
	if err != nil {
		return versionInfo{}, err
	}
	updateClientInfo(client, ver, &info)
	return info, nil
}

func getCoreInfo() (versionInfo,  error) {
	var info versionInfo
	info.CoreServices = version.GetVersion()
	var err error
	info.SupportedConsumerProtocols, err = protocols.GetSupportedProtocolsArray("consumer")
	if err != nil {
		return versionInfo{}, err
	}
	info.SupportedProducerProtocols, err = protocols.GetSupportedProtocolsArray("producer")
	if err != nil {
		return versionInfo{}, err
	}
	return info, nil
}

func updateClientInfo(client string, ver string, info* versionInfo) {
	if client != "" {
		hint, ok := protocols.ValidateProtocol(client, ver)
		if ok {
			info.ClientSupported = "yes"
		} else {
			info.ClientSupported = "no"
		}
		if client == "consumer" {
			info.ClientConsumerProtocol = ver + " (" + hint + ")"
		}
		if client == "producer" {
			info.ClientProducerProtocol = ver + " (" + hint + ")"
		}
	}
}
