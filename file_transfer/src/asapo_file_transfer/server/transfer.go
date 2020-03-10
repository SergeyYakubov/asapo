package server

import (
	log "asapo_common/logger"
	"asapo_common/utils"
	"errors"
	"fmt"
	"net/http"
	"os"
	"path"
)


type fileTransferRequest struct {
	Folder   string
	FileName string
}


func Exists(name string) bool {
	fi, err := os.Stat(name)
	return !os.IsNotExist(err) && !fi.IsDir()
}


func routeFileTransfer(w http.ResponseWriter, r *http.Request) {
	var request fileTransferRequest
	err := utils.ExtractRequest(r,&request)
	fmt.Println(request)
	if err != nil {
		utils.WriteServerError(w,err,http.StatusBadRequest)
		return
	}

	var extraClaim utils.FolderTokenTokenExtraClaim
	if err := utils.JobClaimFromContext(r, &extraClaim); err != nil {
		utils.WriteServerError(w,err,http.StatusInternalServerError)
		return
	}

	if extraClaim.RootFolder!=request.Folder {
		err_txt := "access forbidden for folder "+request.Folder
		log.Error("cannot transfer file: "+err_txt)
		utils.WriteServerError(w,errors.New(err_txt),http.StatusUnauthorized)
		return
	}

	fullName := request.Folder+string(os.PathSeparator)+request.FileName

	if !Exists(fullName) {
		err_txt := "file "+fullName+" does not exist"
		log.Error("cannot transfer file: "+err_txt)
		utils.WriteServerError(w,errors.New(err_txt),http.StatusBadRequest)
		return
	}
	_, file := path.Split(fullName)
	w.Header().Set("Content-Disposition", "attachment; filename=\""+file+"\"")

	log.Debug("Transferring file " + fullName)

	http.ServeFile(w,r, fullName)
}
