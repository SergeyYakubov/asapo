package server

import (
	"net/http"
	"encoding/json"
	"asapo_common/utils"
	"path/filepath"
	"strings"
	log "asapo_common/logger"
	"github.com/pkg/errors"
)

type authorizationRequest struct {
	BeamtimeId string
	OriginHost string
}

func extractRequest(r *http.Request) (request authorizationRequest, err error) {
	decoder := json.NewDecoder(r.Body)
	err = decoder.Decode(&request)
	return
}

func splitHost(hostPort string) string {
	s := strings.Split(hostPort, ":")
	return s[0]
}

func getBeamlineFromIP(ip string) (string, error) {
	host := splitHost(ip)
	lines, err := utils.ReadStringsFromFile(settings.IpBeamlineMappingFolder + string(filepath.Separator) + host)
	if err != nil {
		return "", err
	}

	if len(lines) < 1 || len(lines[0]) == 0 {
		return "", errors.New("file is empty")
	}

	return lines[0], nil
}

func checkBeamtimeExistsInStrings(info beamtimeInfo, lines []string) bool {
	for _, line := range lines {
		words := strings.Fields(line)
		if len(words) < 3 {
			continue
		}
		if words[1] == info.Beamline && words[2] == info.BeamtimeId {
			return true
		}
	}
	return false
}

func beamtimeExists(info beamtimeInfo) bool {
	lines, err := utils.ReadStringsFromFile(settings.BeamtimeBeamlineMappingFile)

	if err != nil || len(lines) < 3 {
		return false
	}
	lines = lines[2:]
	return checkBeamtimeExistsInStrings(info, lines)
}

func authorize(request authorizationRequest) (bool, beamtimeInfo) {
	for _, pair := range settings.AlwaysAllowedBeamtimes {
		if pair.BeamtimeId == request.BeamtimeId {
			return true, pair
		}
	}
	var answer beamtimeInfo

	beamline, err := getBeamlineFromIP(request.OriginHost)
	if err != nil {
		log.Error("cannot find beamline for " + request.OriginHost + " - " + err.Error())
		return false, beamtimeInfo{}
	}

	answer.Beamline = beamline
	answer.BeamtimeId = request.BeamtimeId
	if (!beamtimeExists(answer)) {
		log.Error("cannot authorize beamtime " + answer.BeamtimeId + " for " + request.OriginHost + " in " + answer.Beamline)
		return false, beamtimeInfo{}
	}
	log.Debug("authorized beamtime " + answer.BeamtimeId + " for " + request.OriginHost + " in " + answer.Beamline)

	return true, answer

}

func routeAuthorize(w http.ResponseWriter, r *http.Request) {
	request, err := extractRequest(r)
	if err != nil {
		w.WriteHeader(http.StatusBadRequest)
		return
	}
	ok, beamtimeInfo := authorize(request)
	if (!ok) {
		w.WriteHeader(http.StatusUnauthorized)
		return
	}

	res, err := utils.MapToJson(&beamtimeInfo)
	if err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		return

	}
	w.WriteHeader(http.StatusOK)
	w.Write([]byte(res))
}
