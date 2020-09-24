package ldap_client

type LdapClient interface {
	GetAllowedIpsForBeamline(url string,base string, filter string) ([]string, error)
}
