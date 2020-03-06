package server

import (
	"asapo_common/utils"
	"net/http"
	"time"
	log "asapo_common/logger"
	"errors"
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
	var response folderToken
	response.Token = token
	answer,_ := utils.MapToJson(response)
	return answer
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


func routeFolderToken(w http.ResponseWriter, r *http.Request) {
	var request folderTokenRequest
	err := utils.ExtractRequest(r,&request)
	if err != nil {
		utils.WriteServerError(w,err,http.StatusBadRequest)
		return
	}

	err = checkBeamtimeToken(request)
	if err != nil {
		utils.WriteServerError(w,err,http.StatusUnauthorized)
		return
	}

	token, err := prepareJWTToken(request)
	if err != nil {
		utils.WriteServerError(w,err,http.StatusInternalServerError)
		return
	}

	answer := folderTokenResponce(token)
	w.WriteHeader(http.StatusOK)
	w.Write(answer)
}
