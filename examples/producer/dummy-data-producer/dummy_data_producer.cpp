#include <iostream>
#include <chrono>
#include <vector>
#include <mutex>
#include <thread>
#include <string>
#include <sstream>

#include "asapo_producer.h"


using std::chrono::system_clock;

std::mutex mutex;
int iterations_remained;

struct Args {
    std::string receiver_address;
    std::string beamtime_id;
    std::string stream;
    std::string token;
    size_t number_of_bytes;
    uint64_t iterations;
    uint64_t nthreads;
    uint64_t mode;
    uint64_t timeout_sec;
    uint64_t images_in_set;
};

void PrintCommandArguments(const Args& args) {
    std::cout << "receiver_address: " << args.receiver_address << std::endl
              << "beamtime_id: " << args.beamtime_id << std::endl
              << "Package size: " << args.number_of_bytes / 1000 << "k" << std::endl
              << "iterations: " << args.iterations << std::endl
              << "nthreads: " << args.nthreads << std::endl
              << "mode: " << args.mode << std::endl
              << "timeout: " << args.timeout_sec << std::endl
              << "images in set: " << args.images_in_set << std::endl
              << std::endl;
}

void TryGetStreamAndToken(Args* args) {
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
        args->stream = seglist[1];
    }
    if (seglist.size() > 2) {
        args->token = seglist[2];
    }
    return;

}




void ProcessCommandArguments(int argc, char* argv[], Args* args) {
    asapo::ExitAfterPrintVersionIfNeeded("Dummy Data Producer", argc, argv);
    if (argc != 8 && argc != 9) {
        std::cout <<
                  "Usage: " << argv[0] <<
                  " <destination> <beamtime_id[%<stream>%<token>]> <number_of_byte> <iterations> <nthreads>"
                  " <mode 0 -t tcp, 1 - filesystem> <timeout (sec)> [n images in set (default 1)]"
                  << std::endl;
        exit(EXIT_FAILURE);
    }
    try {
        args->receiver_address = argv[1];
        args->beamtime_id = argv[2];
        TryGetStreamAndToken(args);
        args->number_of_bytes = std::stoull(argv[3]) * 1000;
        args->iterations = std::stoull(argv[4]);
        args->nthreads = std::stoull(argv[5]);
        args->mode = std::stoull(argv[6]);
        args->timeout_sec = std::stoull(argv[7]);
        if (argc == 9) {
            args->images_in_set = std::stoull(argv[8]);
        } else {
            args->images_in_set = 1;
        }
        PrintCommandArguments(*args);
        return;
    } catch(std::exception& e) {
        std::cerr << "Fail to parse arguments" << std::endl;
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    }
}

void ProcessAfterSend(asapo::GenericRequestHeader header, asapo::Error err) {
    mutex.lock();
    iterations_remained--;
    if (err) {
        std::cerr << "File was not successfully send: " << err << std::endl;
        mutex.unlock();
        return;
    }
    mutex.unlock();
}

void ProcessAfterMetaDataSend(asapo::GenericRequestHeader header, asapo::Error err) {
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

asapo::FileData CreateMemoryBuffer(size_t size) {
    return asapo::FileData(new uint8_t[size]);
}


bool SendDummyData(asapo::Producer* producer, size_t number_of_byte, uint64_t iterations, uint64_t images_in_set,
                   const std::string& stream) {

    asapo::Error err;
    if (iterations > 0) { // send wrong meta, for negative integration tests
        err = producer->SendMetaData("bla", &ProcessAfterMetaDataSend);
    } else {
        err = producer->SendMetaData("{\"dummy_meta\":\"test\"}", &ProcessAfterMetaDataSend);
    }
    if (err) {
        std::cerr << "Cannot send metadata: " << err << std::endl;
        return false;
    }

    for(uint64_t i = 0; i < iterations; i++) {
        auto buffer = CreateMemoryBuffer(number_of_byte);
        asapo::EventHeader event_header{i + 1, number_of_byte, std::to_string(i + 1)};
        std::string meta = "{\"user_meta\":\"test" + std::to_string(i + 1) + "\"}";
        if (!stream.empty()) {
            event_header.file_name = stream + "/" + event_header.file_name;
        }
        event_header.user_metadata = std::move(meta);
        if (images_in_set == 1) {
            auto err = producer->SendData(event_header, std::move(buffer), asapo::kDefaultIngestMode, &ProcessAfterSend);
            if (err) {
                std::cerr << "Cannot send file: " << err << std::endl;
                return false;
            }
        } else {
            for (uint64_t id = 0; id < images_in_set; id++) {
                auto buffer = CreateMemoryBuffer(number_of_byte);
                event_header.subset_id = i + 1;
                event_header.subset_size = images_in_set;
                event_header.file_id = id + 1;
                event_header.file_name = std::to_string(i + 1) + "_" + std::to_string(id + 1);
                if (!stream.empty()) {
                    event_header.file_name = stream + "/" + event_header.file_name;
                }
                event_header.user_metadata = meta;
                auto err = producer->SendData(event_header, std::move(buffer), asapo::kDefaultIngestMode, &ProcessAfterSend);
                if (err) {
                    std::cerr << "Cannot send file: " << err << std::endl;
                    return false;
                }
            }
        }
    }
    return true;
}

std::unique_ptr<asapo::Producer> CreateProducer(const Args& args) {
    asapo::Error err;
    auto producer = asapo::Producer::Create(args.receiver_address, args.nthreads,
                                            args.mode == 0 ? asapo::RequestHandlerType::kTcp : asapo::RequestHandlerType::kFilesystem,
                                            asapo::SourceCredentials{args.beamtime_id, args.stream, args.token }, &err);
    if(err) {
        std::cerr << "Cannot start producer. ProducerError: " << err << std::endl;
        exit(EXIT_FAILURE);
    }

    producer->EnableLocalLog(true);
    producer->SetLogLevel(asapo::LogLevel::Info);
    return producer;
}

void WaitThreadsFinished(const Args& args) {
    uint64_t elapsed_ms = 0;
    while (true) {
        mutex.lock();
        if (iterations_remained <= 0) {
            mutex.unlock();
            break;
        }
        mutex.unlock();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        elapsed_ms += 100;
        if (elapsed_ms > args.timeout_sec * 1000) {
            std::cerr << "Producer exit on timeout " << std::endl;
            exit(EXIT_FAILURE);
        }
    }

}

void PrintOutput(const Args& args, const system_clock::time_point& start) {
    system_clock::time_point t2 = system_clock::now();
    double duration_sec = std::chrono::duration_cast<std::chrono::milliseconds>( t2 - start ).count() / 1000.0;
    double size_gb = double(args.number_of_bytes) * args.iterations / 1000.0  / 1000.0 / 1000.0 * 8.0;
    double rate = args.iterations / duration_sec;
    std::cout << "Rate: " << rate << " Hz" << std::endl;
    std::cout << "Bandwidth " << size_gb / duration_sec << " Gbit/s" << std::endl;
}



int main (int argc, char* argv[]) {
    Args args;
    ProcessCommandArguments(argc, argv, &args);

    auto producer = CreateProducer(args);

    iterations_remained = args.iterations * args.images_in_set + 1;

    system_clock::time_point start_time = system_clock::now();

    if(!SendDummyData(producer.get(), args.number_of_bytes, args.iterations, args.images_in_set, args.stream)) {
        return EXIT_FAILURE;
    }

    WaitThreadsFinished(args);
    PrintOutput(args, start_time);

    return EXIT_SUCCESS;
}

