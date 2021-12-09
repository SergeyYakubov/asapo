package cli

import (
	"encoding/json"
	"fmt"
)

func (cmd *command) CommandList_tokens() (err error) {
	message_string := "List tokens"
	if cmd.description(message_string) {
		return nil
	}

	tokens,err := store.GetTokenList()
	if err != nil {
		return err
	}

	answer,_ := json.Marshal(tokens)
	fmt.Fprintf(outBuf, "%s\n", string(answer))
	return nil
}

