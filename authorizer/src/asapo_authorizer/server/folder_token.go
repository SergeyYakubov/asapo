package server

import (
	"asapo_common/utils"
	"net/http"
	"time"
	log "asapo_common/logger"
	"errors"
	"path/filepath"
)

type folderTokenRequest struct {
	Folder string
	BeamtimeId string
	Token      string
}

type folderToken struct {
	Token      string
}

/*func routeFolderToken(w http.ResponseWriter, r *http.Request) {
	utils.ProcessJWTAuth(processFolderTokenRequest,settings.secret)(w,r)
}*/

func prepareJWTToken(request folderTokenRequest) (string,error) {
	var claims utils.CustomClaims
	var extraClaim utils.FolderTokenTokenExtraClaim

	extraClaim.RootFolder = request.Folder
	claims.ExtraClaims = &extraClaim
	claims.Duration = time.Duration(settings.TokenDurationMin) * time.Minute

	return authJWT.GenerateToken(&claims)

}

func folderTokenResponce(token string) []byte{
	return []byte(token)
}

func checkBeamtimeToken(request folderTokenRequest) error {
	token_expect, _ := authHMAC.GenerateToken(&request.BeamtimeId)
	var err_string string
	if request.Token != token_expect {
		err_string = "wrong token for beamtime " + request.BeamtimeId
		log.Error(err_string)
		return errors.New(err_string)
	}
	return nil
}


func extractFolderTokenrequest(r *http.Request) (folderTokenRequest,error) {
	var request folderTokenRequest
	err := utils.ExtractRequest(r,&request)
	if err != nil {
		return folderTokenRequest{},err
	}

	if len(request.Folder)==0 ||len(request.BeamtimeId)==0 || len(request.Token) == 0 {
		return folderTokenRequest{},errors.New("some request fields are empty")
	}
	return request,nil

}

func checkBeamtimeFolder(request folderTokenRequest) error {
	beamtimeMeta, err := findMeta(SourceCredentials{request.BeamtimeId,"auto","",""})
	if err != nil {
		log.Error("cannot get beamtime meta"+err.Error())
		return err
	}

	folder := filepath.Clean(request.Folder)
	if (folder != filepath.Clean(beamtimeMeta.OnlinePath) && folder != filepath.Clean(beamtimeMeta.OfflinePath)) {
		err_string := folder + " does not match beamtime folders "+beamtimeMeta.OnlinePath+" or " +beamtimeMeta.OfflinePath
		log.Error(err_string)
		return errors.New(err_string)
	}

	return nil
}

func routeFolderToken(w http.ResponseWriter, r *http.Request) {
	request, err := extractFolderTokenrequest(r)
	if err != nil {
		utils.WriteServerError(w,err,http.StatusBadRequest)
		return
	}

	err = checkBeamtimeToken(request)
	if err != nil {
		utils.WriteServerError(w,err,http.StatusUnauthorized)
		return
	}

	err = checkBeamtimeFolder(request)
	if err != nil {
		utils.WriteServerError(w,err,http.StatusUnauthorized)
		return
	}

	token, err := prepareJWTToken(request)
	if err != nil {
		utils.WriteServerError(w,err,http.StatusInternalServerError)
		return
	}

	log.Debug("generated folder token for beamtime " + request.BeamtimeId + ", folder " + request.Folder)

	answer := folderTokenResponce(token)
	w.WriteHeader(http.StatusOK)
	w.Write(answer)
}
