package server

import (
	"asapo_authorizer/authorization"
	"asapo_common/structs"
	"asapo_common/utils"
	"encoding/json"
	"fmt"
	"github.com/stretchr/testify/assert"
	"io/ioutil"
	"net/http"
	"testing"
)

var  IntrospectTests = [] struct {
	tokenSubject string
	role string
	status int
	message string
}{
	{"bt_test","read",http.StatusOK,"valid token"},
	{"","",http.StatusUnauthorized,"invalid token"},

}

func TestIntrospect(t *testing.T) {
	authJWT := utils.NewJWTAuth("secret")
	authAdmin := utils.NewJWTAuth("secret_admin")
	authUser := utils.NewJWTAuth("secret_user")
	Auth = authorization.NewAuth(authUser,authAdmin,authJWT)
	for _, test := range IntrospectTests {
		token := prepareUserToken(test.tokenSubject,test.role)
		if test.status==http.StatusUnauthorized {
			token = "blabla"
		}
		request :=  makeRequest(structs.IntrospectTokenRequest{token})
		w := doPostRequest("/introspect",request,"")
		assert.Equal(t, test.status , w.Code, test.message)
		if test.status == http.StatusOK {
			body, _ := ioutil.ReadAll(w.Body)
			var token structs.IntrospectTokenResponse
			json.Unmarshal(body,&token)
			assert.Equal(t, token.Sub , test.tokenSubject, test.message)
			assert.Equal(t, token.AccessType , test.role, test.message)
		} else {
			body, _ := ioutil.ReadAll(w.Body)
			fmt.Println(string(body))
		}
	}
}

