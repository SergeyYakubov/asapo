package server

import (
	"asapo_authorizer/common"
	log "asapo_common/logger"
	"asapo_common/utils"
	"errors"
	"net/http"
	"path/filepath"
	"strings"
)

type SourceCredentials struct {
	BeamtimeId string
	Beamline   string
	Stream     string
	Token      string
	Type 	   string
}

type authorizationRequest struct {
	SourceCredentials string
	OriginHost        string
}

func getSourceCredentials(request authorizationRequest) (SourceCredentials, error) {
	vals := strings.Split(request.SourceCredentials, "%")

	if len(vals) != 5 {
		return SourceCredentials{}, errors.New("cannot get source credentials from " + request.SourceCredentials)
	}
	creds := SourceCredentials{vals[1], vals[2], vals[3], vals[4],vals[0]}
	if creds.Stream == "" {
		creds.Stream = "detector"
	}

	if creds.Beamline == "" {
		creds.Beamline = "auto"
	}

	if creds.BeamtimeId == "" {
		creds.BeamtimeId = "auto"
	}

	if creds.BeamtimeId == "auto" && creds.Beamline == "auto" {
		return SourceCredentials{}, errors.New("cannot automaticaly detect both beamline and beamtime_id ")
	}

	return creds, nil
}


func splitHost(hostPort string) string {
	s := strings.Split(hostPort, ":")
	return s[0]
}

func beamtimeMetaFromJson(fname string) (beamtimeMeta, error) {
	var meta beamtimeMeta
	err := utils.ReadJsonFromFile(fname, &meta)
	if err != nil {
		return beamtimeMeta{}, err
	}
	return meta, nil
}

func beamtimeMetaFromMatch(match string) (beamtimeMeta, error) {
	match = strings.TrimPrefix(match, settings.RootBeamtimesFolder)
	match = strings.TrimPrefix(match, string(filepath.Separator))
	vars := strings.Split(match, string(filepath.Separator))
	if len(vars) != 6 {
		return beamtimeMeta{}, errors.New("bad pattern")
	}
	var bt beamtimeMeta
	ignoredFoldersAfterGpfs := []string{"common", "BeamtimeUsers", "state", "support"}
	if utils.StringInSlice(vars[2], ignoredFoldersAfterGpfs) {
		return beamtimeMeta{}, errors.New("skipped fodler")
	}

	bt.OfflinePath = settings.RootBeamtimesFolder+string(filepath.Separator)+match
	bt.Beamline, bt.BeamtimeId = vars[2], vars[5]

	return bt, nil
}

func findBeamtimeInfoFromId(beamtime_id string) (beamtimeMeta, error) {
	sep := string(filepath.Separator)
	pattern := sep + "*" + sep + "gpfs" + sep + "*" + sep + "*" + sep + "*" + sep
	matches, err := filepath.Glob(settings.RootBeamtimesFolder + pattern + beamtime_id)

	if err != nil || len(matches) == 0 {
		return beamtimeMeta{}, errors.New("Cannot find beamline for "+beamtime_id)
	}

	for _, match := range (matches) {
		btInfo, err := beamtimeMetaFromMatch(match)
		if err != nil {
			continue
		}
		if btInfo.BeamtimeId == beamtime_id {
			return btInfo, nil
		}
	}
	return beamtimeMeta{}, errors.New("Cannot find beamline for "+beamtime_id)
}

func findBeamtimeMetaFromBeamline(beamline string) (beamtimeMeta, error) {
	sep := string(filepath.Separator)
	pattern := "beamtime-metadata-*.json"
	online_path := settings.CurrentBeamlinesFolder + sep + beamline + sep + "current"

	matches, err := filepath.Glob(online_path + sep + pattern)
	if err != nil || len(matches) != 1 {
		return beamtimeMeta{}, err
	}
	meta, err := beamtimeMetaFromJson(matches[0])
	if (err != nil) {
		return beamtimeMeta{}, err
	}
	meta.OnlinePath = online_path
	return meta, nil
}

func alwaysAllowed(creds SourceCredentials) (beamtimeMeta, bool) {
	for _, pair := range settings.AlwaysAllowedBeamtimes {
		if pair.BeamtimeId == creds.BeamtimeId {
			pair.Stream = creds.Stream
			pair.Type = creds.Type
			return pair, true
		}
	}
	return beamtimeMeta{}, false
}

func authorizeByHost(host_ip, beamline string) (error) {
	filter := strings.Replace(settings.Ldap.FilterTemplate,"__BEAMLINE__",beamline,1)
	allowed_ips, err := ldapClient.GetAllowedIpsForBeamline(settings.Ldap.Uri,settings.Ldap.BaseDn, filter)
	if err != nil {
		log.Error("cannot get list of allowed hosts from LDAP: " + err.Error())
		return err
	}

	if (!utils.StringInSlice(splitHost(host_ip),allowed_ips)) {
		err_string := "beamine " +beamline+" not allowed for host " + host_ip
		log.Error(err_string)
		return errors.New(err_string)
	}
	return nil
}

func needHostAuthorization(creds SourceCredentials) bool {
	return creds.Type == "raw" || len(creds.Token) == 0
}

func authorizeByToken(creds SourceCredentials) error {
	var token_expect string
	if (creds.BeamtimeId != "auto") {
		token_expect, _ = authHMAC.GenerateToken(&creds.BeamtimeId)
	} else {
		key := "bl_" + creds.Beamline
		token_expect, _ = authHMAC.GenerateToken(&key)
	}

	var err_string string
	if creds.Token != token_expect {
		if creds.BeamtimeId != "auto" {
			err_string = "wrong token for beamtime " + creds.BeamtimeId
		} else {
			err_string = "wrong token for beamline " + creds.Beamline
		}
		log.Error(err_string)
		return errors.New(err_string)
	}
	return nil
}

func findMeta(creds SourceCredentials) (beamtimeMeta, error) {
	var err error
	var meta beamtimeMeta
	if (creds.BeamtimeId != "auto") {
		meta, err = findBeamtimeInfoFromId(creds.BeamtimeId)
		if (err == nil ) {
			meta_onilne, err_online := findBeamtimeMetaFromBeamline(meta.Beamline)
			if err_online == nil && meta.BeamtimeId == meta_onilne.BeamtimeId {
				meta.OnlinePath = meta_onilne.OnlinePath
			}
		}
	} else {
		meta, err = findBeamtimeMetaFromBeamline(creds.Beamline)
	}

	if creds.Type == "processed" {
		meta.OnlinePath = ""
	}

	if (err != nil) {
		log.Error(err.Error())
		return beamtimeMeta{}, err
	}

	meta.Stream = creds.Stream
	meta.Type = creds.Type

	return meta, nil
}

func authorizeMeta(meta beamtimeMeta, request authorizationRequest, creds SourceCredentials) error {

	if creds.Type=="raw" && meta.OnlinePath=="" {
		err_string := "beamtime "+meta.BeamtimeId+" is not online"
		log.Error(err_string)
		return errors.New(err_string)
	}

	if creds.Beamline != "auto" && meta.Beamline != creds.Beamline {
		err_string := "given beamline (" + creds.Beamline + ") does not match the found one (" + meta.Beamline + ")"
		log.Debug(err_string)
		return errors.New(err_string)
	}

	if needHostAuthorization(creds) {
		if err := authorizeByHost(request.OriginHost, meta.Beamline); err != nil {
			return err
		}
	} else {
		if err := authorizeByToken(creds); err != nil {
			return err
		}
	}

	return nil
}

func authorize(request authorizationRequest, creds SourceCredentials) (beamtimeMeta, error) {
	if meta, ok := alwaysAllowed(creds); ok {
		return meta, nil
	}

	meta, err := findMeta(creds)
	if err != nil {
		return beamtimeMeta{}, err
	}

	if err := authorizeMeta(meta, request, creds); err != nil {
		return beamtimeMeta{}, err
	}

	log.Debug("authorized beamtime " + meta.BeamtimeId + " for " + request.OriginHost + " in " + meta.Beamline+", type "+meta.Type)
	return meta, nil
}

func routeAuthorize(w http.ResponseWriter, r *http.Request) {
	var request authorizationRequest
	err := utils.ExtractRequest(r,&request)
	if err != nil {
		utils.WriteServerError(w,err,http.StatusBadRequest)
		return
	}

	creds, err := getSourceCredentials(request)
	if err != nil {
		utils.WriteServerError(w,err,http.StatusBadRequest)
		return
	}

	beamtimeInfo, err := authorize(request, creds)
	if (err != nil) {
		serr,ok:=err.(*common.ServerError)
		if ok {
			utils.WriteServerError(w,err,serr.Code)
			return
		}
		utils.WriteServerError(w,err,http.StatusUnauthorized)
		return
	}

	res, err := utils.MapToJson(&beamtimeInfo)
	if err != nil {
		utils.WriteServerError(w,err,http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusOK)
	w.Write([]byte(res))
}
