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
	BeamtimeId string
	Beamline   string
	DataSource string
	Token      string
	Type       string
}

type authorizationRequest struct {
	SourceCredentials string
	OriginHost        string
}

func getSourceCredentials(request authorizationRequest) (SourceCredentials, error) {

	vals := strings.Split(request.SourceCredentials, "%")
	nvals := len(vals)
	if nvals < 5 {
		return SourceCredentials{}, errors.New("cannot get source credentials from " + request.SourceCredentials)
	}

	creds := SourceCredentials{Type: vals[0], BeamtimeId: vals[1], Beamline: vals[2], Token: vals[nvals-1]}
	creds.DataSource = strings.Join(vals[3:nvals-1], "%")
	if creds.DataSource == "" {
		creds.DataSource = "detector"
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

func beamtimeMetaFromJson(fname string) (common.BeamtimeMeta, error) {
	var meta common.BeamtimeMeta
	err := utils.ReadJsonFromFile(fname, &meta)
	if err != nil {
		return common.BeamtimeMeta{}, err
	}
	return meta, nil
}

func commissioningMetaFromJson(fname string) (common.BeamtimeMeta, error) {
	var meta common.BeamtimeMeta
	var comMeta common.CommissioningMeta
	err := utils.ReadJsonFromFile(fname, &comMeta)
	if err != nil {
		return common.BeamtimeMeta{}, err
	}
	meta.BeamtimeId = comMeta.Id
	meta.Beamline = strings.ToLower(comMeta.Beamline)
	meta.OfflinePath = comMeta.OfflinePath
	return meta, nil
}

func beamtimeMetaFromMatch(match string) (common.BeamtimeMeta, error) {
	match = strings.TrimPrefix(match, common.Settings.RootBeamtimesFolder)
	match = strings.TrimPrefix(match, string(filepath.Separator))
	vars := strings.Split(match, string(filepath.Separator))
	if len(vars) != 6 {
		return common.BeamtimeMeta{}, errors.New("bad pattern")
	}
	var bt common.BeamtimeMeta
	ignoredFoldersAfterGpfs := []string{"common", "BeamtimeUsers", "state", "support"}
	if utils.StringInSlice(vars[2], ignoredFoldersAfterGpfs) {
		return common.BeamtimeMeta{}, errors.New("skipped fodler")
	}

	bt.OfflinePath = common.Settings.RootBeamtimesFolder + string(filepath.Separator) + match
	bt.Beamline, bt.BeamtimeId = vars[2], vars[5]

	return bt, nil
}

func findBeamtimeInfoFromId(beamtime_id string) (common.BeamtimeMeta, error) {
	cachedMetas.lock.Lock()
	meta, ok := cachedMetas.cache[beamtime_id]
	cachedMetas.lock.Unlock()
	if ok {
		return meta, nil
	}

	sep := string(filepath.Separator)
	pattern := sep + "*" + sep + "gpfs" + sep + "*" + sep + "*" + sep + "*" + sep
	matches, err := filepath.Glob(common.Settings.RootBeamtimesFolder + pattern + beamtime_id)

	if err != nil || len(matches) == 0 {
		return common.BeamtimeMeta{}, errors.New("Cannot find beamline for " + beamtime_id)
	}

	for _, match := range matches {
		btInfo, err := beamtimeMetaFromMatch(match)
		if err != nil {
			continue
		}
		if btInfo.BeamtimeId == beamtime_id {
			cachedMetas.lock.Lock()
			cachedMetas.cache[beamtime_id] = btInfo
			cachedMetas.lock.Unlock()
			return btInfo, nil
		}
	}
	return common.BeamtimeMeta{}, errors.New("Cannot find beamline for " + beamtime_id)
}

func findMetaFileInFolder(beamline string, iscommissioning bool) (string, string, error) {
	sep := string(filepath.Separator)
	var pattern, folder string
	if !iscommissioning {
		pattern = "beamtime-metadata-*.json"
		folder = "current"
	} else {
		pattern = "commissioning-metadata-*.json"
		folder = "commissioning"
	}
	online_path := common.Settings.CurrentBeamlinesFolder + sep + beamline + sep + folder
	matches, err := filepath.Glob(online_path + sep + pattern)
	if err != nil {
		return "", "", err
	}
	if len(matches) != 1 {
		return "", "", errors.New("should be one beamtime-metadata file in folder")
	}
	return matches[0], online_path, nil

}

func findBeamtimeMetaFromBeamline(beamline string, iscommissioning bool) (meta common.BeamtimeMeta, err error) {
	fName, online_path, err := findMetaFileInFolder(beamline, iscommissioning)
	if err != nil {
		return common.BeamtimeMeta{}, err
	}

	if iscommissioning {
		meta, err = commissioningMetaFromJson(fName)
	} else {
		meta, err = beamtimeMetaFromJson(fName)
	}
	if err != nil {
		return common.BeamtimeMeta{}, err
	}

	if meta.BeamtimeId == "" || meta.OfflinePath == "" || meta.Beamline == "" {
		return common.BeamtimeMeta{}, errors.New("cannot set meta fields from beamtime file")
	}

	meta.OnlinePath = online_path
	return meta, nil
}

func alwaysAllowed(creds SourceCredentials) (common.BeamtimeMeta, bool) {
	for _, pair := range common.Settings.AlwaysAllowedBeamtimes {
		if pair.BeamtimeId == creds.BeamtimeId {
			pair.DataSource = creds.DataSource
			pair.Type = creds.Type
			pair.AccessTypes = []string{"read", "write","writeraw"}
			return pair, true
		}
	}
	return common.BeamtimeMeta{}, false
}

func authorizeByHost(host_ip, beamline string) error {
	filter := strings.Replace(common.Settings.Ldap.FilterTemplate, "__BEAMLINE__", beamline, 1)
	allowed_ips, err := ldapClient.GetAllowedIpsForBeamline(common.Settings.Ldap.Uri, common.Settings.Ldap.BaseDn, filter)
	if err != nil {
		log.Error("cannot get list of allowed hosts from LDAP: " + err.Error())
		return err
	}

	if !utils.StringInSlice(splitHost(host_ip), allowed_ips) {
		err_string := "beamine " + beamline + " not allowed for host " + host_ip
		log.Error(err_string)
		return errors.New(err_string)
	}
	return nil
}

func canUseHostAuthorization(creds SourceCredentials) bool {
	return len(creds.Token) == 0
}

func checkTokenRevoked(tokenId string) (err error) {
	revoked, err := store.IsTokenRevoked(tokenId)
	if err != nil {
		return &common.ServerError{utils.StatusServiceUnavailable, err.Error()}
	}
	if revoked {
		return errors.New("token was revoked")
	}
	return nil
}

func checkToken(token string, subject_expect string) (accessTypes []string, err error) {
	var extra_claim structs.AccessTokenExtraClaim
	claim, err := Auth.UserAuth().CheckAndGetContent(token, &extra_claim)
	if err != nil {
		return nil, err
	}

	err = checkTokenRevoked(claim.Id)
	if err != nil {
		return nil, err
	}

	if extra_claim.AccessTypes == nil || len(extra_claim.AccessTypes) == 0 {
		return nil, errors.New("missing access types")
	}

	if claim.Subject != subject_expect {
		return nil, errors.New("wrong token for " + subject_expect)
	}
	return extra_claim.AccessTypes, err
}

func authorizeByToken(creds SourceCredentials) (accessTypes []string, err error) {
	subject_expect := ""
	if creds.BeamtimeId != "auto" {
		subject_expect = utils.SubjectFromBeamtime(creds.BeamtimeId)
	} else {
		subject_expect = utils.SubjectFromBeamline(creds.Beamline)
	}
	return checkToken(creds.Token, subject_expect)
}

func iscommissioning(beamtime string) bool {
	return len(beamtime) > 0 && beamtime[0] == 'c'
}

func findMeta(creds SourceCredentials) (common.BeamtimeMeta, error) {
	var err error
	var meta common.BeamtimeMeta
	if creds.BeamtimeId != "auto" {
		meta, err = findBeamtimeInfoFromId(creds.BeamtimeId)
		if err == nil {
			meta_onilne, err_online := findBeamtimeMetaFromBeamline(meta.Beamline, iscommissioning(creds.BeamtimeId))
			if err_online == nil && meta.BeamtimeId == meta_onilne.BeamtimeId {
				meta.OnlinePath = meta_onilne.OnlinePath
			}
		}
	} else {
		meta, err = findBeamtimeMetaFromBeamline(creds.Beamline, false)
	}

	if creds.Type == "processed" {
		meta.OnlinePath = ""
	}

	if err != nil {
		log.Error(err.Error())
		return common.BeamtimeMeta{}, err
	}

	meta.DataSource = creds.DataSource
	meta.Type = creds.Type

	return meta, nil
}

func authorizeMeta(meta common.BeamtimeMeta, request authorizationRequest, creds SourceCredentials) (accessTypes []string, err error) {
	accessTypes = nil
	if creds.Type == "raw" && meta.OnlinePath == "" {
		err_string := "beamtime " + meta.BeamtimeId + " is not online"
		log.Error(err_string)
		return nil, errors.New(err_string)
	}

	if creds.Beamline != "auto" && meta.Beamline != creds.Beamline {
		err_string := "given beamline (" + creds.Beamline + ") does not match the found one (" + meta.Beamline + ")"
		log.Debug(err_string)
		return nil, errors.New(err_string)
	}

	if canUseHostAuthorization(creds) {
		if err := authorizeByHost(request.OriginHost, meta.Beamline); err != nil {
			return nil, err
		}
		if creds.Type == "raw" {
			accessTypes = []string{"read", "write", "writeraw"}
		} else {
			accessTypes = []string{"read", "write"}
		}
	} else {
		accessTypes, err = authorizeByToken(creds)
	}

	return accessTypes, err
}

func authorize(request authorizationRequest, creds SourceCredentials) (common.BeamtimeMeta, error) {
	if meta, ok := alwaysAllowed(creds); ok {
		return meta, nil
	}

	meta, err := findMeta(creds)
	if err != nil {
		return common.BeamtimeMeta{}, err
	}

	var accessTypes []string
	if accessTypes, err = authorizeMeta(meta, request, creds); err != nil {
		return common.BeamtimeMeta{}, err
	}

	meta.AccessTypes = accessTypes
	log.Debug("authorized creds bl/bt: ", creds.Beamline+"/"+creds.BeamtimeId+", beamtime "+meta.BeamtimeId+" for "+request.OriginHost+" in "+
		meta.Beamline+", type "+meta.Type, "online path "+meta.OnlinePath+", offline path "+meta.OfflinePath)
	return meta, nil
}

func writeServerError(w http.ResponseWriter, err error) {
	serr, ok := err.(*common.ServerError)
	if ok {
		utils.WriteServerError(w, err, serr.Code)
		return
	}
	utils.WriteServerError(w, err, http.StatusUnauthorized)
	return
}

func routeAuthorize(w http.ResponseWriter, r *http.Request) {
	var request authorizationRequest
	err := utils.ExtractRequest(r, &request)
	if err != nil {
		utils.WriteServerError(w, err, http.StatusBadRequest)
		return
	}

	creds, err := getSourceCredentials(request)
	if err != nil {
		utils.WriteServerError(w, err, http.StatusBadRequest)
		return
	}

	beamtimeInfo, err := authorize(request, creds)
	if err != nil {
		writeServerError(w, err)
		return
	}

	res, err := utils.MapToJson(&beamtimeInfo)
	if err != nil {
		utils.WriteServerError(w, err, http.StatusInternalServerError)
		return
	}

	w.WriteHeader(http.StatusOK)
	w.Write(res)
}

func checkRole(w http.ResponseWriter, r *http.Request, role string) error {
	var extraClaim structs.AccessTokenExtraClaim
	var claims *utils.CustomClaims
	if err := utils.JobClaimFromContext(r, &claims, &extraClaim); err != nil {
		w.WriteHeader(http.StatusInternalServerError)
		w.Write([]byte(err.Error()))
		return err
	}

	if err := checkTokenRevoked(claims.Id); err != nil {
		writeServerError(w, err)
		return err
	}

	if claims.Subject != "admin" || !utils.StringInSlice(role, extraClaim.AccessTypes) {
		err_txt := "wrong token claims"
		w.WriteHeader(http.StatusUnauthorized)
		w.Write([]byte(err_txt))
		return errors.New(err_txt)
	}
	return nil
}
