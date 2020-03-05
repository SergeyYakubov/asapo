package server

import (
	"asapo_common/utils"
	"encoding/json"
	"net/http"
	"time"
)

type folderTokenRequest struct {
	Folder string
	BeamtimeId string
	Token      string
}

type folderToken struct {
	Token      string
}

type TokenExtraClaim struct {
	RootFolder string
}

/*func routeFolderToken(w http.ResponseWriter, r *http.Request) {
	utils.ProcessJWTAuth(processFolderTokenRequest,settings.secret)(w,r)
}*/

func extractFolderTokenRequest(r *http.Request) (request folderTokenRequest, err error) {
	decoder := json.NewDecoder(r.Body)
	err = decoder.Decode(&request)
	return
}

func routeFolderToken(w http.ResponseWriter, r *http.Request) {

	request, err := extractFolderTokenRequest(r)
	if err != nil {
		w.WriteHeader(http.StatusBadRequest)
		w.Write([]byte(err.Error()))
		return
	}

	var claims utils.CustomClaims
	var extraClaim TokenExtraClaim

	extraClaim.RootFolder = request.Folder
	claims.ExtraClaims = &extraClaim
	claims.Duration = time.Duration(settings.TokenDurationMin) * time.Minute

	auth := utils.NewJWTAuth(settings.secret)
	token, err := auth.GenerateToken(&claims)
	if err != nil {
		w.WriteHeader(http.StatusBadRequest)
		w.Write([]byte(err.Error()))
		return
	}

	var response folderToken
	response.Token = token
	answer,_ := utils.MapToJson(response)
	w.WriteHeader(http.StatusOK)
	w.Write(answer)
}
