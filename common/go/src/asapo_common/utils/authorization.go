package utils

import (
	"context"
	"crypto/hmac"
	"crypto/sha256"
	"encoding/base64"
	"errors"
	"github.com/dgrijalva/jwt-go"
	"net/http"
	"net/url"
	"strings"
	"time"
)

type AuthorizationRequest struct {
	Token   string
	Command string
	URL     string
}

type AuthorizationResponce struct {
	Status       int
	StatusText   string
	UserName     string
	Token        string
	ValidityTime int
}

type Auth interface {
	GenerateToken(...interface{}) (string, error)
	ProcessAuth(http.HandlerFunc, string) http.HandlerFunc
	Name() string
}


func (a *JWTAuth) Name() string {
	return "Bearer"
}


func stripURL(u *url.URL) string {
	s := u.Path + u.RawQuery
	s = strings.Replace(s, "/", "", -1)
	s = strings.Replace(s, "?", "", -1)
	return s

}

func SplitAuthToken(s string) (authType, token string, err error) {
	keys := strings.Split(s, " ")

	if len(keys) != 2 {
		err = errors.New("authorization error - wrong token")
		return
	}

	authType = keys[0]
	token = keys[1]
	return
}

func ExtractAuthInfo(r *http.Request) (authType, token string, err error) {

	t := r.Header.Get("Authorization")

	if t != "" {
		return SplitAuthToken(t)
	}

	cookie, err := r.Cookie("Authorization")
	if err == nil {
		return SplitAuthToken(cookie.Value)
	}

	err = errors.New("no authorization info")
	return

}

type CustomClaims struct {
	jwt.StandardClaims
	ExtraClaims interface{}
}

func (claim *CustomClaims) SetExpiration(duration time.Duration){
	if duration > 0 {
		claim.ExpiresAt = time.Now().Add(duration).Unix()
	}
}

type JWTAuth struct {
	Key string
}

func NewJWTAuth(key string) *JWTAuth {
	a := JWTAuth{key}
	return &a
}

func (t JWTAuth) GenerateToken(val ...interface{}) (string, error) {
	if len(val) != 1 {
		return "", errors.New("No claims")
	}
	claims, ok := val[0].(*CustomClaims)
	if !ok {
		return "", errors.New("Wrong claims")
	}

	token := jwt.NewWithClaims(jwt.SigningMethodHS256, claims)
	tokenString, err := token.SignedString([]byte(t.Key))

	if err != nil {
		return "", err
	}

	return tokenString, nil
}

func (a *JWTAuth)ProcessAuth(fn http.HandlerFunc, payload string) http.HandlerFunc {
	// payload ignored
	return ProcessJWTAuth(fn,a.Key)
}

func ProcessJWTAuth(fn http.HandlerFunc, key string) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {
		if (r.RequestURI == "/health-check") { // always allow /health-check request
			fn(w,r)
			return
		}
		authType, token, err := ExtractAuthInfo(r)

		if err != nil {
			http.Error(w, err.Error(), http.StatusUnauthorized)
			return
		}

		ctx := r.Context()

		if authType == "Bearer" {
			if claims, ok := CheckJWTToken(token, key); !ok {
				http.Error(w, "Authorization error - tocken does not match", http.StatusUnauthorized)
				return
			} else {
				ctx = context.WithValue(ctx, "TokenClaims", claims)
			}
		} else {
			http.Error(w, "Authorization error - wrong auth type", http.StatusUnauthorized)
			return
		}
		fn(w, r.WithContext(ctx))
	}
}

func CheckJWTToken(token, key string) (jwt.Claims, bool) {

	if token == "" {
		return nil, false
	}

	t, err := jwt.ParseWithClaims(token, &CustomClaims{}, func(token *jwt.Token) (interface{}, error) {
		return []byte(key), nil
	})

	if err == nil && t.Valid {
		return t.Claims, true
	}

	return nil, false
}

func JobClaimFromContext(r *http.Request, val interface{}) error {
	c := r.Context().Value("TokenClaims")

	if c == nil {
		return errors.New("Empty context")
	}

	claim := c.(*CustomClaims)

	return MapToStruct(claim.ExtraClaims.(map[string]interface{}), val)
}

type HMACAuth struct {
	Key string
}

func NewHMACAuth(key string) *HMACAuth {
	a := HMACAuth{key}
	return &a
}

func (a *HMACAuth) Name() string {
	return "HMAC-SHA-256"
}


func generateHMACToken(value string, key string) string {
	mac := hmac.New(sha256.New, []byte(key))
	mac.Write([]byte(value))

	return base64.URLEncoding.EncodeToString(mac.Sum(nil))
	}

func (h HMACAuth) GenerateToken(val ...interface{}) (string, error) {
	if len(val) != 1 {
		return "", errors.New("Wrong claims")
	}
	value, ok := val[0].(*string)
	if !ok {
		return "", errors.New("Wrong claims")
	}

	sha := generateHMACToken(*value, h.Key)

	return sha, nil
}

func (a *HMACAuth)ProcessAuth(fn http.HandlerFunc, payload string) http.HandlerFunc {
	return ProcessHMACAuth(fn,payload,a.Key)
}

func ProcessHMACAuth(fn http.HandlerFunc, payload, key string) http.HandlerFunc {
	return func(w http.ResponseWriter, r *http.Request) {

		authType, token, err := ExtractAuthInfo(r)

		if err != nil {
			http.Error(w, err.Error(), http.StatusUnauthorized)
			return
		}
		if authType == "HMAC-SHA-256" {
			if !CheckHMACToken(payload, token, key) {
				http.Error(w, "Internal authorization error - token does not match", http.StatusUnauthorized)
				return
			}
		} else {
			http.Error(w, "Internal authorization error - wrong auth type", http.StatusUnauthorized)
			return
		}
		fn(w, r)
	}
}

func CheckHMACToken(value string, token, key string) bool {

	if token == "" {
		return false
	}

	generated_token := generateHMACToken(value, key)
	return token == generated_token
}
