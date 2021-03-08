package server

import (
	log "asapo_common/logger"
	"asapo_common/utils"
	"encoding/json"
	"errors"
	"github.com/rs/xid"
	"net/http"
	"time"
)

type userTokenRequest struct {
	BeamtimeId   string
	Beamtimeline string
	DaysValid    int
	Role         string
}

type userToken struct {
	Token      string
	AccessType string
	Expires    string
}

func prepareUserJWTToken(request userTokenRequest) (string, error) {
	var claims utils.CustomClaims
	var extraClaim utils.AccessTokenExtraClaim

	if request.Beamtimeline != "" {
		claims.Subject = "bl_" + request.Beamtimeline
	} else {
		claims.Subject = "bt_" + request.BeamtimeId
	}

	extraClaim.Role = request.Role
	claims.ExtraClaims = &extraClaim
	claims.SetExpiration(time.Duration(request.DaysValid*24) * time.Hour)
	uid := xid.New()
	claims.Id = uid.String()

	return authJWT.GenerateToken(&claims)

}

func extractUserTokenrequest(r *http.Request) (userTokenRequest, error) {
	var request userTokenRequest
	err := utils.ExtractRequest(r, &request)
	if err != nil {
		return userTokenRequest{}, err
	}

	if request.BeamtimeId == "" && request.Beamtimeline == "" {
		return userTokenRequest{}, errors.New("missing beamtime/beamline")
	}

	if request.BeamtimeId != "" && request.Beamtimeline != "" {
		return userTokenRequest{}, errors.New("set only one of beamtime/beamline")
	}

	if request.Role != "read" && request.Role != "write" {
		return userTokenRequest{}, errors.New("wrong role " + request.Role)
	}

	return request, nil
}

func userTokenResponce(request userTokenRequest, token string) []byte {
	answer := userToken{
		Token:      token,
		AccessType: request.Role,
		Expires:    time.Now().Add(time.Duration(request.DaysValid*24) * time.Hour).UTC().Format(time.RFC3339),
	}
	res, _ := json.Marshal(answer)
	return res
}

func authorisedUserToken(w http.ResponseWriter, r *http.Request) {
	authHMACAdmin.ProcessAuth(routeUserToken, "admin")(w, r)
}

func routeUserToken(w http.ResponseWriter, r *http.Request) {
	request, err := extractUserTokenrequest(r)
	if err != nil {
		utils.WriteServerError(w, err, http.StatusBadRequest)
		return
	}

	token, err := prepareUserJWTToken(request)
	if err != nil {
		utils.WriteServerError(w, err, http.StatusInternalServerError)
		return
	}

	log.Debug("generated user token for beamtime " + request.BeamtimeId)

	answer := userTokenResponce(request, token)
	w.WriteHeader(http.StatusOK)
	w.Write(answer)
}
