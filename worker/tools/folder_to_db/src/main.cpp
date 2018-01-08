#include <string>
#include <iostream>

#include "folder_db_importer.h"


struct InputParameters {
    std::string folder;
    std::string uri;
};

InputParameters ProcessCommandArguments(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Usage: " + std::string{argv[0]} +" <path to folder> <database uri>" << std::endl;
        exit(EXIT_FAILURE);
    }

    return InputParameters{argv[1], argv[2]};
}


int main(int argc, char* argv[]) {
    auto command_args = ProcessCommandArguments(argc, argv);

    hidra2::FolderToDbImporter importer;

    auto err = importer.Convert(command_args.folder, command_args.uri);
    if (err != hidra2::FolderToDbImportError::kOK) {
        return 1;
    }

    return 0;
}

