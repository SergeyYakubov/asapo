package ldap_client

import (
	"asapo_authorizer/common"
	"asapo_common/utils"
	"github.com/stretchr/testify/assert"
	"testing"
)

func TestOpenLDAP(t *testing.T) {
	lc := new(OpenLdapClient)
	uri := "ldap://localhost:389"
	base := "ou=rgy,o=desy,c=de"
	filter:= "(cn=a3p00-hosts)"
	expected_ips := []string{"127.0.0.1"}
	res,err := lc.GetAllowedIpsForBeamline(uri,base,filter)
	assert.Nil(t,err)
	assert.Equal(t,expected_ips,res)
}

func TestOpenLDAPCannotDeal(t *testing.T) {
	lc := new(OpenLdapClient)
	uri := "ldap://localhost1:3891"
	base := "ou=rgy,o=desy,c=de"
	filter:= "(cn=a3p00-hosts)"
	_,err := lc.GetAllowedIpsForBeamline(uri,base,filter)
	se,ok:= err.(*common.ServerError)
	assert.Equal(t,ok,true)
	if  ok {
		assert.Equal(t,utils.StatusServiceUnavailable,se.Code)
	}
}
