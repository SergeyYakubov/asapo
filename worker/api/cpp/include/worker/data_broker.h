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
auto const kAuthorizationError = "authorization error";
auto const kInternalError = "Internal Error";
auto const kUnknownIOError = "Unknown IO Error";
}

class DataBroker {
  public:
    //! Connect to the data source - will scan file folders or connect to the database.
// TODO: do we need this?
    virtual Error Connect() = 0;
    //! Reset counter for the specific group.
    /*!
      \param group_id - group id to use.
      \return nullptr of command was successful, otherwise error.
    */
    virtual Error ResetCounter(std::string group_id) = 0;

    //! Set timeout for broker operations. Default - no timeout
    virtual void SetTimeout(uint64_t timeout_ms) = 0;


    //! Get current number of datasets
    /*!
      \param err - return nullptr of operation succeed, error otherwise.
      \return number of datasets.
    */
    virtual uint64_t GetNDataSets(Error* err) = 0;

    //! Generate new GroupID.
    /*!
      \param err - return nullptr of operation succeed, error otherwise.
      \return group ID.
    */
    virtual std::string GenerateNewGroupId(Error* err) = 0;

    //! Get Beamtime metadata.
    /*!
      \param err - return nullptr of operation succeed, error otherwise.
      \return beamtime metadata.
    */
    virtual std::string GetBeamtimeMeta(Error* err) = 0;

    //! Receive next available image.
    /*!
      \param info -  where to store image metadata. Can be set to nullptr only image data is needed.
      \param group_id - group id to use.
      \param data - where to store image data. Can be set to nullptr only image metadata is needed.
      \return Error if both pointers are nullptr or data cannot be read, nullptr otherwise.
    */
    virtual Error GetNext(FileInfo* info, std::string group_id, FileData* data) = 0;


    //! Receive dataset by id.
    /*!
      \param id - dataset id
      \param info -  where to store image metadata. Can be set to nullptr only image data is needed.
      \param data - where to store image data. Can be set to nullptr only image metadata is needed.
      \return Error if both pointers are nullptr or data cannot be read, nullptr otherwise.
    */
    virtual Error GetById(uint64_t id, FileInfo* info, std::string group_id, FileData* data) = 0;


    //! Receive last available image.
    /*!
      \param info -  where to store image metadata. Can be set to nullptr only image data is needed.
      \param group_id - group id to use.
      \param data - where to store image data. Can be set to nullptr only image metadata is needed.
      \return Error if both pointers are nullptr or data cannot be read, nullptr otherwise.
    */
    virtual Error GetLast(FileInfo* info, std::string group_id, FileData* data) = 0;

    //! Get all images matching the query.
    /*!
      \param sql_query -  query string in SQL format. Limit subset is supported
      \param err - will be set in case of error, nullptr otherwise
      \return vector of image metadata matchiing to specified query. Empty if nothing found or error
    */
    virtual FileInfos QueryImages(std::string query,Error* err) = 0;

    virtual ~DataBroker() = default; // needed for unique_ptr to delete itself
};

/*! A class to create a data broker instance. The class's only function Create is used for this*/
class DataBrokerFactory {
  public:
    static std::unique_ptr<DataBroker> CreateFolderBroker(const std::string& source_name,
            Error* error) noexcept;
    static std::unique_ptr<DataBroker> CreateServerBroker(std::string server_name, std::string source_path,
            std::string beamtime_id, std::string token,
            Error* error) noexcept;

};

}
#endif //ASAPO_DATASOURCE_H
