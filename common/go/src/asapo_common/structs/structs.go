package structs

type FolderTokenTokenExtraClaim struct {
	RootFolder string
}

type AccessTokenExtraClaim struct {
	AccessTypes []string
}

type IntrospectTokenRequest struct {
	Token      string
}

type IntrospectTokenResponse struct {
	Sub         string
	AccessTypes []string
}

type IssueTokenRequest struct {
	Subject     map[string]string
	DaysValid   int
	AccessTypes []string
}

type IssueTokenResponse struct {
	Token       string
	Sub         string
	AccessTypes []string
	Expires     string
}
