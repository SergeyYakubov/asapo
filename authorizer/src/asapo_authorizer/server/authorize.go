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
	Beamline string
	Stream string
	Token string
}

type authorizationRequest struct {
	SourceCredentials string
	OriginHost string
}

func getSourceCredentials(request authorizationRequest ) (SourceCredentials,error){
	vals := strings.Split(request.SourceCredentials,"%")

	if len(vals)!=4 {
		return SourceCredentials{}, errors.New("cannot get source credentials from "+request.SourceCredentials)
	}
	creds := SourceCredentials{vals[0],vals[1],vals[2],vals[3]}
	if creds.Stream=="" {
		creds.Stream="detector"
	}

	if creds.Beamline=="" {
		creds.Beamline="auto"
	}

	if creds.BeamtimeId=="" {
		creds.BeamtimeId="auto"
	}

	if creds.BeamtimeId=="auto" && creds.Beamline=="auto" {
		return SourceCredentials{}, errors.New("cannot automaticaly detect both beamline and beamtime_id ")
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

func beamtimeInfoFromMatch(match string) (beamtimeInfo,error) {
	match = strings.TrimPrefix(match, settings.RootBeamtimesFolder)
	match = strings.TrimPrefix(match, string(filepath.Separator))
	vars := strings.Split(match,string(filepath.Separator))
	if len(vars)!=6 {
		return beamtimeInfo{},errors.New("bad pattern")
	}
	var bt beamtimeInfo
	ignoredFoldersAfterGpfs:=[]string{"common","BeamtimeUsers","state","support"}
	if utils.StringInSlice(vars[2],ignoredFoldersAfterGpfs) {
		return beamtimeInfo{},errors.New("skipped fodler")
	}

	bt.Facility,bt.Beamline,bt.Year,bt.BeamtimeId = vars[0],vars[2],vars[3],vars[5]

	return bt,nil
}

func findBeamtime(beamtime_id string) (beamtimeInfo,bool) {
	sep := string(filepath.Separator)
	pattern := sep+"*"+sep+"gpfs"+sep+"*"+sep+"*"+sep+"*"+sep
	matches, err := filepath.Glob(settings.RootBeamtimesFolder+pattern+beamtime_id)

	if err!=nil || len(matches)==0 {
		return beamtimeInfo{},false
	}

	for _,match := range (matches) {
		btInfo,err := beamtimeInfoFromMatch(match)
		if err!= nil {
			continue
		}
		if btInfo.BeamtimeId == beamtime_id {
			return btInfo,true
		}
	}
	return beamtimeInfo{},false
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
		log.Error("wrong token for beamtime " + creds.BeamtimeId)
		return false
	}
	return true
}


func authorize(request authorizationRequest,creds SourceCredentials) (beamtimeInfo,bool) {
	if answer,ok := alwaysAllowed(creds);ok {
		return answer,ok
	}

	beamlineInfo,ok := findBeamtime(creds.BeamtimeId)
	if (!ok) {
		log.Error("cannot find beamline for " + creds.BeamtimeId)
		return beamtimeInfo{},false
	}

	if needHostAuthorization(creds)  {
		if !authorizeByHost(request.OriginHost,beamlineInfo.Beamline) {
			return beamtimeInfo{}, false
		}
	} else {
		if !authorizeByToken(creds) {
			return beamtimeInfo{}, false
		}
	}

	if creds.Beamline !="auto"  && beamlineInfo.Beamline != creds.Beamline{
		log.Debug("given beamline (" + creds.Beamline+") does not match the found one (" +beamlineInfo.Beamline+")" )
		return beamtimeInfo{}, false
	}

	var answer beamtimeInfo
	answer.Beamline = beamlineInfo.Beamline
	answer.Facility = beamlineInfo.Facility
	answer.Year = beamlineInfo.Year
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
