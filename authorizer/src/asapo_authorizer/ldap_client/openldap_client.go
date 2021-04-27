package ldap_client

import (
	"asapo_authorizer/common"
	log "asapo_common/logger"
	"asapo_common/utils"
	"net"
	"strings"
)
import "github.com/go-ldap/ldap"

type OpenLdapClient struct {
}

func (c *OpenLdapClient) GetAllowedIpsForBeamline(url string, base string, filter string) ([]string, error) {
	l, err := ldap.DialURL(url)
	if err != nil {
		return []string{}, &common.ServerError{utils.StatusServiceUnavailable, err.Error()}
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
		if ldap.IsErrorWithCode(err, ldap.LDAPResultNoSuchObject) {
			return []string{}, nil
		} else {
			return []string{}, err
		}
	}

	res := make([]string, 0)
	var lasterr error = nil
	for _, entry := range sr.Entries {
		hosts := entry.GetAttributeValues("nisNetgroupTriple")
		for _, host := range hosts {
			host = strings.TrimPrefix(host, "(")
			host = strings.Split(host, ",")[0]
			addrs, err := net.LookupIP(host)
			if err != nil {
				lasterr = err
				log.Warning("cannot lookup ip for " + host)
				continue
			}
			for _, addr := range addrs {
				if ipv4 := addr.To4(); ipv4 != nil {
					res = append(res, ipv4.String())
				}
			}
		}
	}
	if len(res) == 0 {
		return res, lasterr
	}
	return res, nil
}
