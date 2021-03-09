package authorization

import (
	"asapo_common/utils"
	"encoding/json"
	"github.com/rs/xid"
	"time"
)

type Auth struct {
	authHMAC  utils.Auth
	authAdmin utils.Auth
	authJWT   utils.Auth
}

func NewAuth(authHMAC,authHMACAdmin,authJWT utils.Auth) *Auth {
	return &Auth{authHMAC,authHMACAdmin,authJWT}
}

func (auth *Auth) AdminAuth() utils.Auth {
	return auth.authAdmin
}

func (auth *Auth) HmacAuth() utils.Auth {
	return auth.authHMAC
}

func (auth *Auth) JWTAuth() utils.Auth {
	return auth.authJWT
}

func subjectFromRequest(request TokenRequest) string {
	for key,value := range request.Subject {
		switch key {
		case "beamline":
			return "bl_" + value
		case "beamtimeId":
			return "bt_" + value
		default:
			return value
		}
	}
	return ""
}

func (auth *Auth) PrepareAccessToken(request TokenRequest) (string, error) {
	var claims utils.CustomClaims
	var extraClaim utils.AccessTokenExtraClaim

	claims.Subject = subjectFromRequest(request)

	extraClaim.AccessType = request.AccessType
	claims.ExtraClaims = &extraClaim
	claims.SetExpiration(time.Duration(request.DaysValid*24) * time.Hour)
	uid := xid.New()
	claims.Id = uid.String()

	return auth.authAdmin.GenerateToken(&claims)

}

func UserTokenResponce(request TokenRequest, token string) []byte {
	expires := ""
	if request.DaysValid>0 {
		expires = time.Now().Add(time.Duration(request.DaysValid*24) * time.Hour).UTC().Format(time.RFC3339)
	}
	answer := TokenResponce{
		Token:      token,
		AccessType: request.AccessType,
		Sub:  subjectFromRequest(request),
		Expires:  expires,
	}
	res, _ := json.Marshal(answer)
	return res
}
