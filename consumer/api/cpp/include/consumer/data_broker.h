#ifndef ASAPO_DATASOURCE_H
#define ASAPO_DATASOURCE_H

#include <memory>
#include <string>
#include <vector>

#include "common/data_structs.h"
#include "common/error.h"

namespace asapo {

class DataBroker {
  public:
    //! Reset counter for the specific group.
    /*!
      \param group_id - group id to use.
      \return nullptr of command was successful, otherwise error.
    */
    virtual Error ResetLastReadMarker(std::string group_id) = 0;
    virtual Error ResetLastReadMarker(std::string group_id, std::string substream) = 0;

    virtual Error SetLastReadMarker(uint64_t value, std::string group_id) = 0;
    virtual Error SetLastReadMarker(uint64_t value, std::string group_id, std::string substream) = 0;

    //! Acknowledge data tuple for specific group id and substream.
    /*!
        \param group_id - group id to use.
        \param id - data tuple id
        \param substream (optional) - substream
        \return nullptr of command was successful, otherwise error.
    */
    virtual Error Acknowledge(std::string group_id, uint64_t id, std::string substream = kDefaultSubstream) = 0;

  //! Get unacknowledged tuple for specific group id and substream.
  /*!
      \param group_id - group id to use.
      \param substream (optional) - substream
      \param from_id - return tuples with ids greater or equal to from (use 0 disable limit)
      \param to_id - return tuples with ids less or equal to to (use 0 to disable limit)
      \param in (optional) - substream
      \param err - set to nullptr of operation succeed, error otherwise.
      \return vector of ids, might be empty
  */
    virtual IdList GetUnacknowledgedTupleIds(std::string group_id, std::string substream, uint64_t from_id, uint64_t to_id, Error* error) = 0;
    virtual IdList GetUnacknowledgedTupleIds(std::string group_id, uint64_t from_id, uint64_t to_id, Error* error) = 0;

    //! Set timeout for broker operations. Default - no timeout
    virtual void SetTimeout(uint64_t timeout_ms) = 0;

    //! Set list of substreams
    virtual std::vector<std::string> GetSubstreamList(Error* err) = 0;

    //! Get current number of datasets
    /*!
      \param err - return nullptr of operation succeed, error otherwise.
      \return number of datasets.
    */
    virtual uint64_t GetCurrentSize(Error* err) = 0;
    virtual uint64_t GetCurrentSize(std::string substream, Error* err) = 0;

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
    virtual Error GetNext(FileInfo* info, std::string group_id, std::string substream, FileData* data) = 0;

    //! Retrieves image using fileinfo.
    /*!
      \param info - image metadata to use, can be updated after operation
      \param data - where to store image data. Can be set to nullptr only image metadata is needed.
      \return Error if data is nullptr or data cannot be read, nullptr otherwise.
    */
    virtual Error RetrieveData(FileInfo* info, FileData* data) = 0;


    //! Receive next available completed dataset.
    /*!
      \param err -  will be set to error data cannot be read, nullptr otherwise.
      \param group_id - group id to use.
      \return DataSet - information about the dataset
    */
    virtual DataSet GetNextDataset(std::string group_id, Error* err) = 0;
    virtual DataSet GetNextDataset(std::string group_id, std::string substream, Error* err) = 0;

    //! Receive last available completed dataset.
    /*!
      \param err -  will be set to error data cannot be read, nullptr otherwise.
      \param group_id - group id to use.
      \return DataSet - information about the dataset
    */
    virtual DataSet GetLastDataset(std::string group_id, Error* err) = 0;
    virtual DataSet GetLastDataset(std::string group_id, std::string substream, Error* err) = 0;

    //! Receive dataset by id.
    /*!
      \param id - dataset id
      \param err -  will be set to error data cannot be read or dataset is incomplete, nullptr otherwise.
      \param group_id - group id to use.
      \return DataSet - information about the dataset
    */
    virtual DataSet GetDatasetById(uint64_t id, std::string group_id, Error* err) = 0;
    virtual DataSet GetDatasetById(uint64_t id, std::string group_id, std::string substream, Error* err) = 0;

    //! Receive single image by id.
    /*!
      \param id - image id
      \param info -  where to store image metadata. Can be set to nullptr only image data is needed.
      \param data - where to store image data. Can be set to nullptr only image metadata is needed.
      \return Error if both pointers are nullptr or data cannot be read, nullptr otherwise.
    */
    virtual Error GetById(uint64_t id, FileInfo* info, std::string group_id, FileData* data) = 0;
    virtual Error GetById(uint64_t id, FileInfo* info, std::string group_id, std::string substream, FileData* data) = 0;

  //! Receive id of last acknowledged data tuple
  /*!
    \param group_id - group id to use.
    \param substream (optional) - substream
    \param err -  will be set in case of error, nullptr otherwise.
    \return id of the last acknowledged image, 0 if error
  */
    virtual uint64_t GetLastAcknowledgedTulpeId(std::string group_id, std::string substream, Error* error) = 0;
    virtual uint64_t GetLastAcknowledgedTulpeId(std::string group_id, Error* error) = 0;

    //! Receive last available image.
    /*!
      \param info -  where to store image metadata. Can be set to nullptr only image data is needed.
      \param group_id - group id to use.
      \param data - where to store image data. Can be set to nullptr only image metadata is needed.
      \return Error if both pointers are nullptr or data cannot be read, nullptr otherwise.
    */
    virtual Error GetLast(FileInfo* info, std::string group_id, FileData* data) = 0;
    virtual Error GetLast(FileInfo* info, std::string group_id, std::string substream, FileData* data) = 0;

    //! Get all images matching the query.
    /*!
      \param sql_query -  query string in SQL format. Limit subset is supported
      \param err - will be set in case of error, nullptr otherwise
      \return vector of image metadata matchiing to specified query. Empty if nothing found or error
    */
    virtual FileInfos QueryImages(std::string query, Error* err) = 0;
    virtual FileInfos QueryImages(std::string query, std::string substream, Error* err) = 0;

  //! Configure resending nonacknowledged data
  /*!
    \param resend -  where to resend
    \param resend_after - how many seconds to wait for acknowledgment
    \param resend_attempts - how many time to resend.
  */
  virtual void SetResendNacs(bool resend, uint64_t resend_after, uint64_t resend_attempts) = 0;


    virtual ~DataBroker() = default; // needed for unique_ptr to delete itself
};

/*! A class to create a data broker instance. The class's only function Create is used for this*/
class DataBrokerFactory {
  public:
    static std::unique_ptr<DataBroker> CreateServerBroker(std::string server_name, std::string source_path,
            bool has_filesystem,
            SourceCredentials source,
            Error* error) noexcept;

};

}
#endif //ASAPO_DATASOURCE_H
