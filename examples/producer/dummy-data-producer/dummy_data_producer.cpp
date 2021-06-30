#include <iostream>
#include <iostream>
#include <chrono>
#include <vector>
#include <mutex>
#include <thread>
#include <string>
#include <sstream>

#include "asapo/asapo_producer.h"
#include "asapo/preprocessor/definitions.h"

using std::chrono::system_clock;

std::mutex mutex;
uint64_t iterations_remained;

struct Args {
    std::string discovery_service_endpoint;
    std::string beamtime_id;
    std::string data_source;
    std::string token;
    size_t number_of_bytes;
    uint64_t iterations;
    uint8_t nthreads;
    uint64_t mode;
    uint64_t timeout_ms;
    uint64_t messages_in_set;
};

void PrintCommandArguments(const Args& args) {
    std::cout << "discovery_service_endpoint: " << args.discovery_service_endpoint << std::endl
              << "beamtime_id: " << args.beamtime_id << std::endl
              << "Package size: " << args.number_of_bytes / 1000 << "k" << std::endl
              << "iterations: " << args.iterations << std::endl
              << "nthreads: " << args.nthreads << std::endl
              << "mode: " << args.mode << std::endl
              << "Write files: " << ((args.mode % 100) / 10 == 1) << std::endl
              << "Tcp mode: " << ((args.mode % 10) == 0 ) << std::endl
              << "Raw: " << (args.mode / 100 == 1) << std::endl
              << "timeout: " << args.timeout_ms << std::endl
              << "messages in set: " << args.messages_in_set << std::endl
              << std::endl;
}

void TryGetDataSourceAndToken(Args* args) {
    std::stringstream test(args->beamtime_id);
    std::string segment;
    std::vector<std::string> seglist;

    while(std::getline(test, segment, '%')) {
        seglist.push_back(segment);
    }
    if (seglist.size() == 1) {
        return;
    }
    if (seglist.size() > 1) {
        args->beamtime_id = seglist[0];
        args->data_source = seglist[1];
    }
    if (seglist.size() > 2) {
        args->token = seglist[2];
    }
    return;

}




void ProcessCommandArguments(int argc, char* argv[], Args* args) {
    if (argc != 8 && argc != 9) {
        std::cout <<
                  "Usage: " << argv[0] <<
                  " <destination> <beamtime_id[%<data_source>%<token>]> <number_of_kbyte> <iterations> <nthreads>"
                  " <mode 0xx - processed source type, 1xx - raw source type, xx0 -t tcp, xx1 - filesystem, x0x - write files, x1x - do not write files> <timeout (sec)> [n messages in set (default 1)]"
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    try {
        args->discovery_service_endpoint = argv[1];
        args->beamtime_id = argv[2];
        TryGetDataSourceAndToken(args);
        args->number_of_bytes = std::stoull(argv[3]) * 1000;
        args->iterations = std::stoull(argv[4]);
        args->nthreads = static_cast<uint8_t>(std::stoi(argv[5]));
        args->mode = std::stoull(argv[6]);
        args->timeout_ms = std::stoull(argv[7]) * 1000;
        if (argc == 9) {
            args->messages_in_set = std::stoull(argv[8]);
        } else {
            args->messages_in_set = 1;
        }
        PrintCommandArguments(*args);
        return;
    } catch(std::exception& e) {
        std::cerr << "Fail to parse arguments" << std::endl;
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ProcessAfterSend(asapo::RequestCallbackPayload, asapo::Error err) {
    mutex.lock();
    iterations_remained--;
    if (err) {
        std::cerr << "File was not successfully send: " << err << std::endl;
        mutex.unlock();
        return;
    }
    mutex.unlock();
}

void ProcessAfterMetaDataSend(asapo::RequestCallbackPayload, asapo::Error err) {
    mutex.lock();
    iterations_remained--;
    if (err) {
        std::cerr << "Metadata was not successfully send: " << err << std::endl;
    } else {
        std::cout << "Metadata was successfully send" << std::endl;
    }
    mutex.unlock();
    return;
}

asapo::MessageData CreateMemoryBuffer(size_t size) {
    return asapo::MessageData(new uint8_t[size]);
}


bool SendDummyData(asapo::Producer* producer, size_t number_of_byte, uint64_t iterations, uint64_t messages_in_set,
                   const std::string& data_source, bool write_files, asapo::SourceType type) {

    asapo::Error err;
    if (iterations == 0) {
        auto mode = asapo::MetaIngestMode{asapo::MetaIngestOp::kReplace, true};
        err = producer->SendBeamtimeMetadata("{\"dummy_meta\":\"test\"}", mode, &ProcessAfterMetaDataSend);
        if (err) {
            std::cerr << "Cannot send metadata: " << err << std::endl;
            return false;
        }
    }

    std::string message_folder = GetStringFromSourceType(type) + asapo::kPathSeparator;


    for (uint64_t i = 0; i < iterations; i++) {
        auto buffer = CreateMemoryBuffer(number_of_byte);
        asapo::MessageHeader message_header{i + 1, number_of_byte, std::to_string(i + 1)};
        std::string meta = "{\"user_meta\":\"test" + std::to_string(i + 1) + "\"}";
        if (!data_source.empty()) {
            message_header.file_name = data_source + "/" + message_header.file_name;
        }
        message_header.file_name = message_folder + message_header.file_name;
        message_header.user_metadata = std::move(meta);
        if (messages_in_set == 1) {
            auto err = producer->Send(message_header,
                                      std::move(buffer),
                                      write_files ? asapo::kDefaultIngestMode :
                                      asapo::kTransferData,
                                      "default",
                                      &ProcessAfterSend);
            if (err) {
                std::cerr << "Cannot send file: " << err << std::endl;
                return false;
            }
        } else {
            for (uint64_t id = 0; id < messages_in_set; id++) {
                auto buffer = CreateMemoryBuffer(number_of_byte);
                message_header.dataset_substream = id + 1;
                message_header.dataset_size = messages_in_set;
                message_header.message_id = i + 1;
                message_header.file_name = std::to_string(i + 1) + "_" + std::to_string(id + 1);
                if (!data_source.empty()) {
                    message_header.file_name = data_source + "/" + message_header.file_name;
                }
                message_header.file_name = message_folder + message_header.file_name;
                message_header.user_metadata = meta;
                auto err =
                    producer->Send(message_header,
                                   std::move(buffer),
                                   write_files ? asapo::kDefaultIngestMode :
                                   asapo::kTransferData,
                                   "default",
                                   &ProcessAfterSend);
                if (err) {
                    std::cerr << "Cannot send file: " << err << std::endl;
                    return false;
                }
            }
        }
    }
    return producer->SendStreamFinishedFlag("default", iterations, "", nullptr) == nullptr;
}

std::unique_ptr<asapo::Producer> CreateProducer(const Args& args) {
    asapo::Error err;
    auto producer = asapo::Producer::Create(args.discovery_service_endpoint, args.nthreads,
                                            args.mode % 10 == 0 ? asapo::RequestHandlerType::kTcp : asapo::RequestHandlerType::kFilesystem,
                                            asapo::SourceCredentials{args.mode / 100 == 0 ? asapo::SourceType::kProcessed : asapo::SourceType::kRaw, args.beamtime_id, "", args.data_source, args.token },
                                            3600000, &err);
    if(err) {
        std::cerr << "Cannot start producer. ProducerError: " << err << std::endl;
        exit(EXIT_FAILURE);
    }

    producer->EnableLocalLog(true);
    producer->SetLogLevel(asapo::LogLevel::Info);
    return producer;
}

void PrintOutput(const Args& args, const system_clock::time_point& start) {
    system_clock::time_point t2 = system_clock::now();
    double duration_sec = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(t2 - start).count())
        / 1000.0;
    double size_gb = static_cast<double>(args.number_of_bytes*args.iterations) / 1000.0 / 1000.0 / 1000.0 * 8.0;
    double rate = static_cast<double>(args.iterations) / duration_sec;
    std::cout << "Rate: " << rate << " Hz" << std::endl;
    std::cout << "Bandwidth " << size_gb / duration_sec << " Gbit/s" << std::endl;
    std::cout << "Bandwidth " << size_gb / duration_sec / 8 << " GBytes/s" << std::endl;
}

int main (int argc, char* argv[]) {
    Args args;
    ProcessCommandArguments(argc, argv, &args);

    auto producer = CreateProducer(args);

    if (args.iterations == 0) {
        iterations_remained = 1; // metadata
    } else {
        iterations_remained = args.iterations * args.messages_in_set;
    }

    system_clock::time_point start_time = system_clock::now();

    if(!SendDummyData(producer.get(), args.number_of_bytes, args.iterations, args.messages_in_set, args.data_source,
                      (args.mode % 100) / 10 == 0, args.mode / 100 == 0 ? asapo::SourceType::kProcessed : asapo::SourceType::kRaw)) {
        return EXIT_FAILURE;
    }

    auto err = producer->WaitRequestsFinished(args.timeout_ms);
    if (err) {
        std::cerr << "Producer exit on timeout " << std::endl;
        exit(EXIT_FAILURE);
    }

    if (iterations_remained != 0) {
        std::cerr << "Producer did not send all data " << std::endl;
        exit(EXIT_FAILURE);
    }
    PrintOutput(args, start_time);

    return EXIT_SUCCESS;
}

