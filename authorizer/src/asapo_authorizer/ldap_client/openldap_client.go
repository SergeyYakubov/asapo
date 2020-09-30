package ldap_client

import (
	"asapo_authorizer/common"
	"asapo_common/utils"
	"net"
	"strings"
)
import "github.com/go-ldap/ldap"

type OpenLdapClient struct {
}

func (c *OpenLdapClient) GetAllowedIpsForBeamline(url string,base string,filter string) ([]string, error) {
	l, err := ldap.DialURL(url)
	if err != nil {
		return []string{},&common.ServerError{utils.StatusServiceUnavailable, err.Error()}
	}
	defer l.Close()

	searchRequest := ldap.NewSearchRequest(
		base,
		ldap.ScopeWholeSubtree, ldap.NeverDerefAliases, 0, 0, false,
		filter,
		[]string{"nisNetgroupTriple"},
		nil,
	)

	sr, err := l.Search(searchRequest)
	if err != nil {
		if ldap.IsErrorWithCode(err,ldap.LDAPResultNoSuchObject) {
			return []string{},nil
		} else {
			return []string{},err
		}
	}

	res := make([]string,0)
	for _, entry := range sr.Entries {
		host := entry.GetAttributeValue("nisNetgroupTriple")
		host = strings.TrimPrefix(host,"(")
		host = strings.Split(host, ",")[0]
		addrs,err := net.LookupIP(host)

		if err != nil {
			return []string{},err
		}
		for _, addr := range addrs {
			if ipv4 := addr.To4(); ipv4 != nil {
				res = append(res,ipv4.String())
			}
		}
	}
	return res,nil
}
