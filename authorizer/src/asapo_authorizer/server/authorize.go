package server

import (
	log "asapo_common/logger"
	"asapo_common/utils"
	"encoding/json"
	"errors"
	"net/http"
	"path/filepath"
	"strings"
)

type SourceCredentials struct {
	BeamtimeId string
	Stream string
	Token string
}

type authorizationRequest struct {
	SourceCredentials string
	OriginHost string
}

func getSourceCredentials(request authorizationRequest ) (SourceCredentials,error){
	vals := strings.Split(request.SourceCredentials,"%")

	if len(vals)!=3 {
		return SourceCredentials{}, errors.New("cannot get source credentials from "+request.SourceCredentials)
	}
	creds := SourceCredentials{vals[0],vals[1],vals[2]}
	if creds.Stream=="" {
		creds.Stream="detector"
	}
	return creds,nil
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

func checkBeamtimeExistsInStrings(beamtime_id string, lines []string) (string,bool) {
	for _, line := range lines {
		words := strings.Fields(line)
		if len(words) < 3 {
			continue
		}
		if words[2] == beamtime_id {
			return words[1], true
		}
	}
	return "",false
}

func beamtimeRegistered(beamtime_id string) (string,bool) {
	lines, err := utils.ReadStringsFromFile(settings.BeamtimeBeamlineMappingFile)

	if err != nil || len(lines) < 3 {
		return "",false
	}
	lines = lines[2:]
	return checkBeamtimeExistsInStrings(beamtime_id, lines)
}

func alwaysAllowed(creds SourceCredentials)(beamtimeInfo,bool) {
	for _, pair := range settings.AlwaysAllowedBeamtimes {
		if pair.BeamtimeId == creds.BeamtimeId {
			pair.Stream = creds.Stream
			return pair,true
		}
	}
	return beamtimeInfo{},false
}

func authorizeByHost(host,beamline string) (bool) {
	active_beamline, err := getBeamlineFromIP(host)
	if err != nil {
		log.Error("cannot find active beamline for " + host + " - " + err.Error())
		return false
	}

	if (active_beamline != beamline) {
		log.Error("beamine for host " + host +" - "+ active_beamline+ " does not match " + beamline)
		return false
	}
	return true
}

func needHostAuthorization(creds SourceCredentials) bool {
	return strings.HasPrefix(creds.Stream,"detector") || len(creds.Token)==0
}

func authorizeByToken(creds SourceCredentials) bool {
	token_expect, _ := auth.GenerateToken(&creds.BeamtimeId)

	if creds.Token != token_expect {
		log.Error("wrong token for beamtime" + creds.BeamtimeId)
		return false
	}
	return true
}


func authorize(request authorizationRequest,creds SourceCredentials) (beamtimeInfo,bool) {
	if answer,ok := alwaysAllowed(creds);ok {
		return answer,ok
	}

	beamline,ok :=beamtimeRegistered(creds.BeamtimeId)
	if (!ok) {
		log.Error("cannot find beamline for " + creds.BeamtimeId)
		return beamtimeInfo{},false
	}

	if needHostAuthorization(creds)  {
		if !authorizeByHost(request.OriginHost,beamline) {
			return beamtimeInfo{}, false
		}
	} else {
		if !authorizeByToken(creds) {
			return beamtimeInfo{}, false
		}
	}

	var answer beamtimeInfo
	answer.Beamline = beamline
	answer.BeamtimeId = creds.BeamtimeId
	answer.Stream = creds.Stream

	log.Debug("authorized beamtime " + answer.BeamtimeId + " for " + request.OriginHost + " in " + answer.Beamline)

	return answer,true

}

func routeAuthorize(w http.ResponseWriter, r *http.Request) {
	request, err := extractRequest(r)
	if err != nil {
		w.WriteHeader(http.StatusBadRequest)
		w.Write([]byte(err.Error()))
		return
	}


	creds,err := getSourceCredentials(request)
	if err!=nil {
		w.WriteHeader(http.StatusBadRequest)
		w.Write([]byte(err.Error()))
		return
	}

	beamtimeInfo,ok := authorize(request,creds)
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
