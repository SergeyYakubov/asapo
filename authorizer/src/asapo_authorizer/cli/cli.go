// Package contains asapo commands that can be executed from command line.
// Every CommandXxxx function that is a member of a cmd struct processes asapo xxxx command
package cli

import (
	"asapo_authorizer/token_store"
	"errors"
	"flag"
	"fmt"
	"io"
	"os"
	"reflect"
	"strings"
)

var flHelp bool

var outBuf io.Writer = os.Stdout

var store token_store.Store

func printHelp(f *flag.FlagSet) bool {
	if flHelp {
		f.Usage()
		return true
	} else {
		return false
	}
}

// DoCommand takes command name as a parameter and executes corresponding to this name cmd method
func DoCommand(name string, args []string) error {
	commandName := "Command" + strings.ToUpper(name[:1]) + strings.ToLower(name[1:])
	cmd := new(command)
	commandName = strings.ReplaceAll(commandName,"-","_")
	methodVal := reflect.ValueOf(cmd).MethodByName(commandName)
	if !methodVal.IsValid() {
		return errors.New("wrong "+ProgramName+" command: " + name + "\nType '"+os.Args[0]+" -help'")
	}
	cmd.name = strings.ReplaceAll(name,"-","_")
	cmd.args = args

	method := methodVal.Interface().(func() error)

	store = new(token_store.TokenStore)
	store.Init(nil)
	defer store.Close()

	return method()
}

// PrintAllCommands prints all available commands (found wihtin methods of cmd)
func PrintAllCommands() {
	fmt.Fprintln(outBuf, "\nCommands:")
	cmd := new(command)
	CmdType := reflect.TypeOf(cmd)
	for i := 0; i < CmdType.NumMethod(); i++ {
		methodVal := CmdType.Method(i)
		if strings.HasPrefix(methodVal.Name, "Command") {
			method := methodVal.Func.Interface().(func(*command) error)
			cmd.name = strings.ToLower(methodVal.Name)[7:]
			cmd.name = strings.ReplaceAll(cmd.name,"_","-")
			cmd.args = []string{"description"}
			method(cmd)
		}
	}
}
