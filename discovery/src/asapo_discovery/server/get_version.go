package server

import (
	"asapo_common/logger"
	"asapo_common/utils"
	"asapo_common/version"
	"asapo_discovery/protocols"
	"encoding/json"
	"errors"
	"net/http"
)

type versionInfo struct {
	SoftwareVersion            string `json:"softwareVersion"`
	ClientProtocol     protocols.ProtocolInfo `json:"clientProtocol"`
	ClientSupported            string `json:"clientSupported"`
	SupportedProtocols []protocols.ProtocolInfo `json:"supportedProtocols"`
}

func extractProtocol(r *http.Request) (string, error) {
	keys := r.URL.Query()
	protocol := keys.Get("protocol")
	if protocol == "" {
		return "", errors.New("cannot extract protocol from request")
	}
	return protocol, nil
}

func routeGetVersion(w http.ResponseWriter, r *http.Request) {
	log_str := "processing get version"
	logger.Debug(log_str)

	if ok := checkDiscoveryApiVersion(w, r); !ok {
		return
	}
	keys := r.URL.Query()
	client := keys.Get("client")
	protocol := keys.Get("protocol")
	info, err := getVersionInfo(client, protocol)
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(err.Error()))
		logger.Error(log_str + " - " + err.Error())
		return
	}
	resp, _ := json.Marshal(&info)
	w.Write(resp)
}

func checkDiscoveryApiVersion(w http.ResponseWriter, r *http.Request) bool {
	_, ok := utils.PrecheckApiVersion(w, r, version.GetDiscoveryApiVersion())
	return ok
}

func getVersionInfo(client string, ver string) (versionInfo, error) {
	info, err := getCoreInfo(client)
	if err != nil {
		return versionInfo{}, err
	}
	if ver=="" {
		return info, nil
	}
	updateClientInfo(client, ver, &info)
	return info, nil
}

func getCoreInfo(client string) (versionInfo, error) {
	var info versionInfo
	info.SoftwareVersion = version.GetVersion()
	if client=="" {
		return info, nil
	}
	var err error
	info.SupportedProtocols, err = protocols.GetSupportedProtocolsArray(client)
	if err != nil {
		return versionInfo{}, err
	}
	return info, nil
}

func updateClientInfo(client string, ver string, info *versionInfo) {
	if client == "" {
		return
	}
	pInfo,valid := getProtocolInfo(client, ver, info)
	setSupported(valid, info)
	if client == "consumer" {
		info.ClientProtocol = pInfo
	} else
	if client == "producer" {
		info.ClientProtocol = pInfo
	}
}

func setSupported(valid bool, info *versionInfo) {
	if valid {
		info.ClientSupported = "yes"
	} else {
		info.ClientSupported = "no"
	}
}

func getProtocolInfo(client string, ver string, info *versionInfo) (pInfo protocols.ProtocolInfo, valid bool) {
	protocol, err := protocols.FindProtocol(client, ver)
	if err != nil {
		pInfo.VersionInfo = ver + " (" + err.Error() + ")"
		valid = false
	} else {
		var hint string
		hint, valid = protocol.IsValid()
		pInfo.VersionInfo = ver + " (" + hint + ")"
		pInfo.MicroservicesApi = protocol.MicroserviceAPis
	}
	return
}
