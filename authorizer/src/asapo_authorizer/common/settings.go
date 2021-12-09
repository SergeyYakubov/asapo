package common

var Settings authorizerSettings

type authorizerSettings struct {
	Port                   int
	LogLevel               string
	RootBeamtimesFolder    string
	CurrentBeamlinesFolder string
	AlwaysAllowedBeamtimes []BeamtimeMeta
	UserSecretFile         string
	AdminSecretFile        string
	FolderTokenDurationMin int
	Ldap                   struct {
		Uri string
		BaseDn string
		FilterTemplate string
	}
	DiscoveryServer string
	DatabaseServer string
	UpdateRevokedTokensIntervalSec int
	UpdateTokenCacheIntervalSec int
}
