#include <string>
#include <iostream>

#include "folder_db_importer.h"


void Usage(char* cmd) {
    std::cout << "Usage: " + std::string{cmd} + "[-i] <path to folder> <database uri>" << std::endl;
    exit(EXIT_FAILURE);
}


hidra2::ConvertParameters ProcessCommandArguments(int argc, char* argv[]) {
    if (argc < 3) {
        Usage(argv[0]);
    }

    int ignore_flag = (std::string(argv[1]) == "-i");

    if (argc != 3 + ignore_flag) {
        Usage(argv[0]);
    }
    hidra2::ConvertParameters params;
    params.ignore_duplicates = ignore_flag;
    params.folder = argv[1 + ignore_flag];
    params.uri = argv[2 + ignore_flag];
    return params;
}

int main(int argc, char* argv[]) {
    auto import_params = ProcessCommandArguments(argc, argv);

    hidra2::FolderToDbImporter importer;
    hidra2::FolderImportStatistics statistics;

    auto err = importer.Convert(import_params, &statistics);
    if (err != hidra2::FolderToDbImportError::kOK) {
        std::cout << "Error import to database" << std::endl;
        return 1;
    }

    std::cout << statistics << std::endl;

    return 0;
}
