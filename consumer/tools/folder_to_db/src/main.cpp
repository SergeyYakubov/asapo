#include <string>
#include <iostream>

#include "folder_db_importer.h"


void Usage(char* cmd) {
    std::cout << "Usage: " + std::string{cmd} +
              " [-i] [-n <number of tasks>] <path to folder> <database name> <database uri>" << std::endl;
    exit(EXIT_FAILURE);
}

struct ConvertParameters {
    bool ignore_duplicates{false};
    std::string folder;
    std::string db_name;
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
        }
    } else {
        Usage(argv[0]);
    }
}


ConvertParameters ProcessCommandArguments(int argc, char* argv[]) {
    if (argc < 4) {
        Usage(argv[0]);
    }
    ConvertParameters params;
    int narg = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            ProcessFlags(&params, argc, &i, argv);
        } else {
            switch (narg) {
            case 0:
                params.folder = argv[i];
                break;
            case 1:
                params.db_name = argv[i];
                break;
            case 2:
                params.uri = argv[i];
                break;
            }
            narg++;
        }
    }

    if (narg != 3) {
        Usage(argv[0]);
    }
    return params;
}

int main(int argc, char* argv[]) {
    auto import_params = ProcessCommandArguments(argc, argv);

    asapo::FolderToDbImporter importer;
    importer.SetNParallelTasks(static_cast<unsigned int>(import_params.ntasks));
    importer.IgnoreDuplicates(import_params.ignore_duplicates);

    asapo::FolderImportStatistics statistics;

    auto err = importer.Convert(import_params.uri, import_params.folder, import_params.db_name,
                                &statistics);
    if (err != nullptr) {
        std::cout << "Error import to database: " << err->Explain() << std::endl;
        return EXIT_FAILURE;
    }

    std::cout << statistics << std::endl;

    return 0;
}
