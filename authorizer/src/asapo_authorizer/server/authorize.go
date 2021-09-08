package server

import (
	"asapo_authorizer/common"
	log "asapo_common/logger"
	"asapo_common/structs"
	"asapo_common/utils"
	"errors"
	"net/http"
	"path/filepath"
	"strings"
)

type SourceCredentials struct {
	Type 	   		string
	BeamtimeId 		string
	Beamline   		string
	DataSource     	string
	Token      		string

	// Optional
	InstanceId		string
	PipelineStep	string
}

type authorizationRequest struct {
	SourceCredentials string
	OriginHost        string
	NewFormat 		  bool
}

func getSourceCredentials(request authorizationRequest) (SourceCredentials, error) {
	vals := strings.Split(request.SourceCredentials, "%")
	nvals:=len(vals)
	if (request.NewFormat && nvals < 7) || (!request.NewFormat && nvals < 5) {
		return SourceCredentials{}, errors.New("cannot get source credentials from " + request.SourceCredentials)
	}

	var creds SourceCredentials

	if request.NewFormat {
		creds = SourceCredentials{
			Type:       vals[0],
			InstanceId: vals[1], PipelineStep: vals[2],
			BeamtimeId: vals[3], Beamline: vals[4], Token: vals[nvals-1]}
		creds.DataSource=strings.Join(vals[5:nvals-1],"%")
	} else {
		creds = SourceCredentials{
			Type:       vals[0],
			InstanceId: "Unset", PipelineStep: "Unset",
			BeamtimeId: vals[1], Beamline: vals[2], Token: vals[nvals-1]}
		creds.DataSource=strings.Join(vals[3:nvals-1],"%")
	}
	if creds.DataSource == "" {
		creds.DataSource = "detector"
	}

	if creds.Beamline == "" {
		creds.Beamline = "auto"
	}

	if creds.BeamtimeId == "" {
		creds.BeamtimeId = "auto"
	}

	if creds.InstanceId == "" {
		creds.InstanceId = "auto"
	}

	if creds.PipelineStep == "" {
		creds.PipelineStep = "auto"
	}

	if creds.InstanceId == "auto" || creds.PipelineStep == "auto" {
		return SourceCredentials{}, errors.New("InstanceId and PipelineStep must be already set on client side")
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
	if err != nil {
		return beamtimeMeta{}, err
	}
	if len(matches) != 1 {
		return beamtimeMeta{}, errors.New("more than one beamtime-metadata file in folder")
	}

	meta, err := beamtimeMetaFromJson(matches[0])
	if (err != nil) {
		return beamtimeMeta{}, err
	}
	if meta.BeamtimeId == "" || meta.OfflinePath=="" || meta.Beamline == ""{
		return beamtimeMeta{}, errors.New("cannot set meta fields from beamtime file")
	}

	meta.OnlinePath = online_path
	return meta, nil
}

func alwaysAllowed(creds SourceCredentials) (beamtimeMeta, bool) {
	for _, pair := range settings.AlwaysAllowedBeamtimes {
		if pair.BeamtimeId == creds.BeamtimeId {
			pair.InstanceId = creds.InstanceId
			pair.PipelineStep = creds.PipelineStep
			pair.DataSource = creds.DataSource
			pair.Type = creds.Type
			pair.AccessTypes = []string{"read","write"}
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

func checkToken(token string, subject_expect string) (accessTypes []string, err error) {
	var extra_claim structs.AccessTokenExtraClaim
	subject,err := Auth.UserAuth().CheckAndGetContent(token,&extra_claim)
	if err!=nil {
		return nil,err
	}

	if extra_claim.AccessTypes==nil || len(extra_claim.AccessTypes)==0 {
		return nil,errors.New("missing access types")
	}

	if subject!=subject_expect {
		return nil,errors.New("wrong token for "+subject_expect)
	}
	return extra_claim.AccessTypes,err
}

func authorizeByToken(creds SourceCredentials) (accessTypes []string, err error) {
	subject_expect:=""
	if (creds.BeamtimeId != "auto") {
		subject_expect = utils.SubjectFromBeamtime(creds.BeamtimeId)
	} else {
		subject_expect = utils.SubjectFromBeamline(creds.Beamline)
	}
	return checkToken(creds.Token,subject_expect)
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

	meta.InstanceId = creds.InstanceId
	meta.PipelineStep = creds.PipelineStep
	meta.DataSource = creds.DataSource
	meta.Type = creds.Type

	return meta, nil
}

func authorizeMeta(meta beamtimeMeta, request authorizationRequest, creds SourceCredentials) (accessTypes []string, err error) {
	accessTypes = nil
	if creds.Type=="raw" && meta.OnlinePath=="" {
		err_string := "beamtime "+meta.BeamtimeId+" is not online"
		log.Error(err_string)
		return nil,errors.New(err_string)
	}

	if creds.Beamline != "auto" && meta.Beamline != creds.Beamline {
		err_string := "given beamline (" + creds.Beamline + ") does not match the found one (" + meta.Beamline + ")"
		log.Debug(err_string)
		return nil,errors.New(err_string)
	}

	if needHostAuthorization(creds) {
		if err := authorizeByHost(request.OriginHost, meta.Beamline); err != nil {
			return nil,err
		}
		accessTypes = []string{"read","write"}
	} else {
		accessTypes,err = authorizeByToken(creds)
	}

	return accessTypes,err
}

func authorize(request authorizationRequest, creds SourceCredentials) (beamtimeMeta, error) {
	if meta, ok := alwaysAllowed(creds); ok {
		return meta, nil
	}

	meta, err := findMeta(creds)
	if err != nil {
		return beamtimeMeta{}, err
	}

	var accessTypes []string
	if accessTypes, err = authorizeMeta(meta, request, creds); err != nil {
		return beamtimeMeta{}, err
	}

	meta.AccessTypes = accessTypes
	log.Debug("authorized creds inst/step: " , creds.InstanceId+"/"+creds.PipelineStep, " bl/bt: ", creds.Beamline+"/"+creds.BeamtimeId+", beamtime " + meta.BeamtimeId + " for " + request.OriginHost + " in " +
		meta.Beamline+", type "+meta.Type, "online path "+meta.OnlinePath + ", offline path "+meta.OfflinePath)
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
