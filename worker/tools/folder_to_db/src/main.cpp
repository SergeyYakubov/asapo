#include <string>
#include <iostream>

#include "folder_db_importer.h"


void Usage(char* cmd) {
    std::cout << "Usage: " + std::string{cmd} + " [-i] [-n <number of tasks>] <path to folder> <database uri>" << std::endl;
    exit(EXIT_FAILURE);
}

struct ConvertParameters {
    bool ignore_duplicates{false};
    std::string folder;
    std::string uri;
    int ntasks{1};
};


void ProcessFlags(ConvertParameters* params, int argc, int* narg, char* argv[]) {
    if (std::string(argv[*narg]) == "-i") {
        params->ignore_duplicates = true;
    } else if (std::string(argv[*narg]) == "-n") {
        if ((*narg + 1) == argc) {
            Usage(argv[0]);
        }
        try {
            params->ntasks = std::stoi(argv[*narg + 1]);
            *narg += 1;
            if (params->ntasks <= 0) {
                Usage(argv[0]);
            }
        } catch (...) {
            Usage(argv[0]);
        };
    } else {
        Usage(argv[0]);
    }
}


ConvertParameters ProcessCommandArguments(int argc, char* argv[]) {
    if (argc < 3) {
        Usage(argv[0]);
    }
    ConvertParameters params;
    int narg = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            ProcessFlags(&params, argc, &i, argv);
        } else {
            narg == 0 ? params.folder = argv[i] : params.uri = argv[i];
            narg++;
        }
    }

    if (narg != 2) {
        Usage(argv[0]);
    }
    return params;
}

int main(int argc, char* argv[]) {
    auto import_params = ProcessCommandArguments(argc, argv);

    hidra2::FolderToDbImporter importer;
    importer.RunInParallel(import_params.ntasks);
    importer.IgnoreDuplicates(import_params.ignore_duplicates);

    hidra2::FolderImportStatistics statistics;

    auto err = importer.Convert(import_params.uri, import_params.folder, &statistics);
    if (err != hidra2::FolderToDbImportError::kOK) {
        std::cout << "Error import to database" << std::endl;
        return 1;
    }

    std::cout << statistics << std::endl;

    return 0;
}
