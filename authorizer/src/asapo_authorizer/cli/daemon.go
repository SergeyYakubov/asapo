package cli

import (
	"asapo_authorizer/server"
)


func (cmd *command) CommandDaemon() error {

	message_string := "Start daemon (default)"
	if cmd.description(message_string) {
		return nil
	}

	server.Start()

	return nil
}
