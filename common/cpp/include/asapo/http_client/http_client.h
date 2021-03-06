#ifndef ASAPO_HTTP_CLIENT_H
#define ASAPO_HTTP_CLIENT_H

#include "asapo/common/error.h"
#include "asapo/common/data_structs.h"

namespace asapo {

enum class HttpCode;

class HttpClient {
  public:
    virtual std::string Get(const std::string& uri, HttpCode* response_code, Error* err) const noexcept = 0;
    virtual std::string Post(const std::string& uri, const std::string& cookie, const std::string& data,
                             HttpCode* response_code,
                             Error* err) const noexcept = 0;
    virtual Error Post(const std::string& uri, const std::string& cookie,
                       const std::string& input_data, MessageData* ouput_data,
                       uint64_t output_data_size,
                       HttpCode* response_code)  const noexcept = 0;
    virtual Error Post(const std::string& uri, const std::string& cookie,
                       const std::string& input_data, std::string output_file_name,
                       HttpCode* response_code)  const noexcept = 0;
    virtual std::string UrlEscape(const std::string& uri) const noexcept = 0;
    virtual ~HttpClient() = default;
};

std::unique_ptr<HttpClient> DefaultHttpClient();

enum class HttpCode : int {
    Continue           = 100,
    SwitchingProtocols = 101,
    Processing         = 102,

    OK                          = 200,
    Created                     = 201,
    Accepted                    = 202,
    NonAuthoritativeInformation = 203,
    NoContent                   = 204,
    ResetContent                = 205,
    PartialContent              = 206,
    MultiStatus                 = 207,
    IMUsed                      = 226,

    MultipleChoices   = 300,
    MovedPermanently  = 301,
    Found             = 302,
    SeeOther          = 303,
    NotModified       = 304,
    UseProxy          = 305,
    TemporaryRedirect = 307,
    PermanentRedirect = 308,

    BadRequest                  = 400,
    Unauthorized                = 401,
    PaymentRequired             = 402,
    Forbidden                   = 403,
    NotFound                    = 404,
    MethodNotAllowed            = 405,
    NotAcceptable               = 406,
    ProxyAuthenticationRequired = 407,
    RequestTimeout              = 408,
    Conflict                    = 409,
    Gone                        = 410,
    LengthRequired              = 411,
    PreconditionFailed          = 412,
    PayloadTooLarge             = 413,
    URITooLong                  = 414,
    UnsupportedMediaType        = 415,
    RangeNotSatisfiable         = 416,
    ExpectationFailed           = 417,
    ImATeapot                   = 418,
    UnprocessableEntity         = 422,
    Locked                      = 423,
    FailedDependency            = 424,
    UpgradeRequired             = 426,
    PreconditionRequired        = 428,
    TooManyRequests             = 429,
    RequestHeaderFieldsTooLarge = 431,
    UnavailableForLegalReasons  = 451,

    InternalServerError           = 500,
    NotImplemented                = 501,
    BadGateway                    = 502,
    ServiceUnavailable            = 503,
    GatewayTimeout                = 504,
    HTTPVersionNotSupported       = 505,
    VariantAlsoNegotiates         = 506,
    InsufficientStorage           = 507,
    NetworkAuthenticationRequired = 511
};


}

#endif //ASAPO_HTTP_CLIENT_H
