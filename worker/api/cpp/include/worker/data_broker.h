#ifndef ASAPO_DATASOURCE_H
#define ASAPO_DATASOURCE_H

#include <memory>
#include <string>

#include "common/data_structs.h"
#include "common/error.h"

namespace asapo {

namespace WorkerErrorMessage {

auto const kMemoryError = "Memory Error";
auto const kEmptyDatasource = "Empty Data Source";
auto const kSourceNotFound = "Source Not Found";
auto const kSourceNotConnected = "Source Not Connacted";
auto const kSourceAlreadyConnected = "Source Already Connected";
auto const kErrorReadingSource = "Error Reading Source";
auto const kNotFound = "Uri not found";
auto const kPermissionDenied = "Permissionn Denied";
auto const kNoData = "No Data";
auto const kWrongInput = "Wrong Input";
auto const kInternalError = "Internal Error";
auto const kUnknownIOError = "Unknown IO Error";
}

class DataBroker {
  public:
    //! Connect to the data source - will scan file folders or connect to the database.
// TODO: do we need this?
    virtual Error Connect() = 0;
    //! Set timeout for broker operations. Default - no timeout
    virtual void SetTimeout(uint64_t timeout_ms) = 0;
    //! Receive next image.
    /*!
      \param info -  where to store image metadata. Can be set to nullptr only image data is needed.
      \param data - where to store image data. Can be set to nullptr only image metadata is needed.
      \return Error if both pointers are nullptr or data cannot be read, WorkerErrorCode::OK otherwise.
    */
    virtual Error GetNext(FileInfo* info, FileData* data) = 0;
    virtual ~DataBroker() = default; // needed for unique_ptr to delete itself
};

/*! A class to create a data broker instance. The class's only function Create is used for this*/
class DataBrokerFactory {
  public:
    static std::unique_ptr<DataBroker> CreateFolderBroker(const std::string& source_name,
            Error* error) noexcept;
    static std::unique_ptr<DataBroker> CreateServerBroker(const std::string& server_name,
            const std::string& source_name,
            Error* error) noexcept;

};

}
#endif //ASAPO_DATASOURCE_H
