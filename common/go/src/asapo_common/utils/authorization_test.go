package utils

import (
	"net/http"
	"testing"
	"net/http/httptest"
	"time"
	"github.com/stretchr/testify/assert"
)

type authorizationResponse struct {
	Status       int
	StatusText   string
	UserName     string
	Token        string
	ValidityTime int
}

type JobClaim struct {
	authorizationResponse
	JobInd string
}


func writeAuthResponse(w http.ResponseWriter, r *http.Request) {
	w.WriteHeader(http.StatusOK)
	var jc JobClaim
	JobClaimFromContext(r,nil,&jc)
	w.Write([]byte(jc.UserName))
	w.Write([]byte(jc.JobInd))
}

func TestGenerateJWTToken(t *testing.T) {

	a := NewJWTAuth("hi")
	cc := CustomClaims{ExtraClaims: nil}
	cc.SetExpiration(0)
	token, _ := a.GenerateToken((&cc))
	assert.Equal(t, "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJFeHRyYUNsYWltcyI6bnVsbH0.QXaiODT7V1tEwmVKCLfpH2WbgjNJpqJcNgeVivFm7GY",
		token, "jwt token")

}

var JWTAuthtests = []struct {
	Mode       string
	Key        string
	User       string
	jobID      string
	Duration   time.Duration
	Answercode int
	Message    string
}{
	{"header", "hi", "testuser", "123", time.Hour, http.StatusOK, "correct auth - header"},
	{"cookie", "hi", "testuser", "123", time.Hour, http.StatusOK, "correct auth - cookie"},
	{"header", "hi", "testuser", "123", time.Microsecond, http.StatusUnauthorized, "token expired"},
	{"header", "hih", "testuser", "123", 1, http.StatusUnauthorized, "wrong key"},
	{"", "hi", "testuser", "123", 1, http.StatusUnauthorized, "auth no header"},
}

func TestProcessJWTAuth(t *testing.T) {
	for _, test := range JWTAuthtests {
		req, _ := http.NewRequest("POST", "http://blabla", nil)

		var claim JobClaim
		claim.UserName = test.User
		claim.JobInd = test.jobID

		a := NewJWTAuth(test.Key)

		cc:= CustomClaims{ExtraClaims: &claim}
		cc.SetExpiration(test.Duration)
		token, _ := a.GenerateToken((&cc))
		if test.Mode == "header" {
			req.Header.Add("Authorization", "Bearer "+token)
		}

		if test.Mode == "cookie" {
			c := http.Cookie{Name: "Authorization", Value: "Bearer "+token}
			req.AddCookie(&c)
		}

		w := httptest.NewRecorder()
		if test.Duration == time.Microsecond {
			if testing.Short() {
				continue
			}
			time.Sleep(time.Second)
		}
		ProcessJWTAuth(http.HandlerFunc(writeAuthResponse), "hi")(w, req)
		assert.Equal(t, test.Answercode, w.Code, test.Message)
		if w.Code == http.StatusOK {
			assert.Contains(t, w.Body.String(), test.User, test.Message)
			assert.Contains(t, w.Body.String(), test.jobID, test.Message)
		}
	}
}

