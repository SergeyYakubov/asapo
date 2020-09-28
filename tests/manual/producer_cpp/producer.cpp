#include <thread>
#include <chrono>
#include "asapo_producer.h"


void ProcessAfterSend(asapo::RequestCallbackPayload payload, asapo::Error err) {
    if (err) {
        std::cerr << "error/warning during send: " << err << std::endl;
        return;
    } else {
        std::cout << "successfuly send " << payload.original_header.Json() << std::endl;
        return;
    }
}

void exit_if_error(std::string error_string, const asapo::Error& err) {
    if (err) {
        std::cerr << error_string << err << std::endl;
        //exit(EXIT_FAILURE);
    }
}

std::string format_string(uint32_t in, std::string format="%05d")
{
    if(in > 99999)
        in = 0;

    char buf[6];
    snprintf(buf,sizeof(buf),format.c_str(),in);
    return std::string(buf);

}


int main(int argc, char* argv[]) {

    uint32_t submodule = 1;
    uint32_t sleeptime = 1;


    if(argc >= 2)
        submodule = atoi(argv[1]);

    if(argc >=3)
        sleeptime = atoi(argv[2]);


    asapo::Error err;

    auto endpoint = "localhost:8400"; // or your endpoint
    auto beamtime = "asapo_test";

    auto producer = asapo::Producer::Create(endpoint, 1,asapo::RequestHandlerType::kTcp,
                                            asapo::SourceCredentials{asapo::SourceType::kProcessed,beamtime, "", "", ""}, 60, &err);
    exit_if_error("Cannot start producer", err);

    uint32_t eventid = 1;
    uint32_t start_number = 1;

    // number of files per acquistion per module
    const uint32_t number_of_splitted_files = 5;

    // number of modules
    const uint32_t modules = 3;

    while(true)
    {
        for(uint32_t part=1; part<=number_of_splitted_files; ++part)
        {
            std::string to_send = "processed/lambdatest_"
                + format_string(start_number) // file start number (acquistion id)
                + "_part" + format_string(part) // file part id (chunk id)
                + "_m" + format_string(submodule, std::string("%02d"));
            auto send_size = to_send.size() + 1;
            auto buffer =  asapo::FileData(new uint8_t[send_size]);
            memcpy(buffer.get(), to_send.c_str(), send_size);
            std::string substream = std::to_string(start_number);
            // std::cout<<"submodule:"<<submodule
            //          <<"- substream:"<<substream
            //          <<"- filename:"<<to_send<<std::endl;

            asapo::EventHeader event_header{submodule, send_size, to_send,"", part,modules};
            // err = producer->SendData(event_header,substream, std::move(buffer),
            //                          asapo::kTransferMetaDataOnly, &ProcessAfterSend);

            err = producer->SendData(event_header,substream, std::move(buffer),
                                     asapo::kDefaultIngestMode, &ProcessAfterSend);
            exit_if_error("Cannot send file", err);

            err = producer->WaitRequestsFinished(1000);
            exit_if_error("Producer exit on timeout", err);
            std::this_thread::sleep_for (std::chrono::seconds(sleeptime));

            // if(part == number_of_splitted_files)
            // {

            //     err = producer->SendSubstreamFinishedFlag(substream,
            //                                               part,
            //                                               std::to_string(start_number+1),
            //                                               &ProcessAfterSend);
            //     exit_if_error("Cannot send file", err);
            // }

        }
        start_number++;

    }


    return EXIT_SUCCESS;
}
