package ldap_client

import "github.com/stretchr/testify/mock"

type MockedLdapClient struct {
	mock.Mock
}

func (c *MockedLdapClient) GetAllowedIpsForBeamline(url string,base string,filter string) ([]string, error) {
	args := c.Called(url,base,filter)
	return args.Get(0).([]string), args.Error(1)
}
