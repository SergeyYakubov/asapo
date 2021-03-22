package authorization

import (
	"asapo_common/structs"
	"asapo_common/utils"
	"encoding/json"
	"github.com/rs/xid"
	"time"
)

type Auth struct {
	authUser  utils.Auth
	authAdmin utils.Auth
	authJWT   utils.Auth
}

func NewAuth(authUser,authAdmin,authJWT utils.Auth) *Auth {
	return &Auth{authUser,authAdmin,authJWT}
}

func (auth *Auth) AdminAuth() utils.Auth {
	return auth.authAdmin
}

func (auth *Auth) UserAuth() utils.Auth {
	return auth.authUser
}

func (auth *Auth) JWTAuth() utils.Auth {
	return auth.authJWT
}

func subjectFromRequest(request structs.IssueTokenRequest) string {
	for key,value := range request.Subject {
		switch key {
		case "beamline":
			return utils.SubjectFromBeamline(value)
		case "beamtimeId":
			return utils.SubjectFromBeamtime(value)
		default:
			return value
		}
	}
	return ""
}

func (auth *Auth) PrepareAccessToken(request structs.IssueTokenRequest, userToken bool) (string, error) {
	var claims utils.CustomClaims
	var extraClaim structs.AccessTokenExtraClaim

	claims.Subject = subjectFromRequest(request)

	extraClaim.AccessTypes = request.AccessTypes
	claims.ExtraClaims = &extraClaim
	claims.SetExpiration(time.Duration(request.DaysValid*24) * time.Hour)
	uid := xid.New()
	claims.Id = uid.String()

	if userToken {
		return auth.UserAuth().GenerateToken(&claims)
	} else {
		return auth.AdminAuth().GenerateToken(&claims)
	}
}

func UserTokenResponce(request structs.IssueTokenRequest, token string) []byte {
	expires := ""
	if request.DaysValid>0 {
		expires = time.Now().Add(time.Duration(request.DaysValid*24) * time.Hour).UTC().Format(time.RFC3339)
	}
	answer := structs.IssueTokenResponse{
		Token:       token,
		AccessTypes: request.AccessTypes,
		Sub:         subjectFromRequest(request),
		Expires:     expires,
	}
	res, _ := json.Marshal(answer)
	return res
}
