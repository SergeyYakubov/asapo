#ifndef HIDRA2_DATASOURCE_H
#define HIDRA2_DATASOURCE_H

#include <memory>
#include <string>
#include <common/file_info.h>

namespace hidra2 {

enum class WorkerErrorCode {
    kOK,
    kMemoryError,
    kEmptyDatasource,
    kSourceNotFound,
    kSourceNotConnected,
    kSourceAlreadyConnected,
    kErrorReadingSource,
    kPermissionDenied,
    kNoData,
    kWrongInput,
    kUnknownIOError
};

class DataBroker {
  public:
    //! Connect to the data source - will scan file folders or connect to the database.
    virtual WorkerErrorCode Connect() = 0;
    //! Receive next image.
    /*!
      \param info -  where to store image metadata. Can be set to nullptr only image data is needed.
      \param data - where to store image data. Can be set to nullptr only image metadata is needed.
      \return Error if both pointers are nullptr or data cannot be read, WorkerErrorCode::OK otherwise.
    */
    virtual WorkerErrorCode GetNext(FileInfo* info, FileData* data) = 0;
    virtual ~DataBroker() = default; // needed for unique_ptr to delete itself
};

/*! A class to create a data broker instance. The class's only function Create is used for this*/
class DataBrokerFactory {
  public:
    static std::unique_ptr<DataBroker> Create(const std::string& source_name, WorkerErrorCode* return_code) noexcept;
};

}
#endif //HIDRA2_DATASOURCE_H
