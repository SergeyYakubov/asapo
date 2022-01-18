package server

import (
	"asapo_authorizer/common"
	log "asapo_common/logger"
	"asapo_common/structs"
	"asapo_common/utils"
	"asapo_common/version"
	"errors"
	"net/http"
	"path/filepath"
	"time"
)

type folderTokenRequest struct {
	Folder     string
	BeamtimeId string
	Token      string
}

type tokenFolders struct {
	RootFolder string
	SecondFolder  string
}

type folderToken struct {
	Token string
}

/*func routeFolderToken(w http.ResponseWriter, r *http.Request) {
	utils.ProcessJWTAuth(processFolderTokenRequest,common.Settings.secret)(w,r)
}*/

func prepareJWTToken(folders tokenFolders) (string, error) {
	var claims utils.CustomClaims
	var extraClaim structs.FolderTokenTokenExtraClaim

	extraClaim.RootFolder = folders.RootFolder
	extraClaim.SecondFolder = folders.SecondFolder

	claims.ExtraClaims = &extraClaim
	claims.SetExpiration(time.Duration(common.Settings.FolderTokenDurationMin) * time.Minute)
	return Auth.JWTAuth().GenerateToken(&claims)

}

func folderTokenResponce(token string) []byte {
	return []byte(token)
}

func checkBeamtimeToken(request folderTokenRequest) error {
	_, err := checkToken(request.Token, utils.SubjectFromBeamtime(request.BeamtimeId))
	return err
}

func extractFolderTokenrequest(r *http.Request) (folderTokenRequest, error) {
	var request folderTokenRequest
	err := utils.ExtractRequest(r, &request)
	if err != nil {
		return folderTokenRequest{}, err
	}

	if len(request.Folder) == 0 || len(request.BeamtimeId) == 0 || len(request.Token) == 0 {
		return folderTokenRequest{}, errors.New("some request fields are empty")
	}
	return request, nil

}

func checkBeamtimeFolder(request folderTokenRequest, ver utils.VersionNum) (folders tokenFolders, err error) {
	beamtimeMeta, err := findMeta(SourceCredentials{request.BeamtimeId, "auto", "", "", ""})
	if err != nil {
		log.Error("cannot get beamtime meta" + err.Error())
		return folders,err
	}

	if request.Folder=="auto" && ver.Id > 1 {
		folders.RootFolder = beamtimeMeta.OfflinePath
		folders.SecondFolder = beamtimeMeta.OnlinePath
		return folders,nil
	}

	folder := filepath.Clean(request.Folder)
	if folder != filepath.Clean(beamtimeMeta.OnlinePath) && folder != filepath.Clean(beamtimeMeta.OfflinePath) {
		err_string := folder + " does not match beamtime folders " + beamtimeMeta.OnlinePath + " or " + beamtimeMeta.OfflinePath
		log.Error(err_string)
		return folders,errors.New(err_string)
	}

	folders.RootFolder = request.Folder
	return folders,nil
}

func checkAuthorizerApiVersion(w http.ResponseWriter, r *http.Request) (utils.VersionNum, bool) {
	return utils.PrecheckApiVersion(w, r, version.GetAuthorizerApiVersion())
}

func routeFolderToken(w http.ResponseWriter, r *http.Request) {
	ver,ok := checkAuthorizerApiVersion(w, r);
	if !ok {
		return
	}

	request, err := extractFolderTokenrequest(r)
	if err != nil {
		utils.WriteServerError(w, err, http.StatusBadRequest)
		return
	}

	err = checkBeamtimeToken(request)
	if err != nil {
		utils.WriteServerError(w, err, http.StatusUnauthorized)
		return
	}

	folders,err := checkBeamtimeFolder(request,ver)
	if err != nil {
		utils.WriteServerError(w, err, http.StatusUnauthorized)
		return
	}

	token, err := prepareJWTToken(folders)
	if err != nil {
		utils.WriteServerError(w, err, http.StatusInternalServerError)
		return
	}

	log.WithFields(map[string]interface{}{
		"folder":request.Folder,
		"beamtime":request.BeamtimeId,
	}).Debug("issued folder token")

	answer := folderTokenResponce(token)
	w.WriteHeader(http.StatusOK)
	w.Write(answer)
}
