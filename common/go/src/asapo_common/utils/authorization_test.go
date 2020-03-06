package utils

import (
	"net/http"
	"testing"
	"net/http/httptest"
	"time"
	"github.com/stretchr/testify/assert"
)

type JobClaim struct {
	AuthorizationResponce
	JobInd string
}


func writeAuthResponse(w http.ResponseWriter, r *http.Request) {
	w.WriteHeader(http.StatusOK)
	var jc JobClaim
	JobClaimFromContext(r, &jc)
	w.Write([]byte(jc.UserName))
	w.Write([]byte(jc.JobInd))
}

func TestGenerateJWTToken(t *testing.T) {

	a := NewJWTAuth("hi")
	token, _ := a.GenerateToken((&CustomClaims{Duration: 0, ExtraClaims: nil}))
	assert.Equal(t, "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJEdXJhdGlvbiI"+
		"6MCwiRXh0cmFDbGFpbXMiOm51bGx9.JJcqNZciIDILk-A2sJZCY1sND458bcjNv6tXC2jxric",
		token, "jwt token")

}

var HJWTAuthtests = []struct {
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
	for _, test := range HJWTAuthtests {
		req, _ := http.NewRequest("POST", "http://blabla", nil)

		var claim JobClaim
		claim.UserName = test.User
		claim.JobInd = test.jobID

		a := NewJWTAuth(test.Key)

		token, _ := a.GenerateToken((&CustomClaims{Duration: test.Duration, ExtraClaims: &claim}))
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

