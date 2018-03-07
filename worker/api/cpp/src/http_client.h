#ifndef HIDRA2_HTTP_CLIENT_H
#define HIDRA2_HTTP_CLIENT_H

#include <hidra2_worker.h>

namespace hidra2 {

enum class HttpCode;

class HttpClient {
  public:
    virtual std::string Get(const std::string& uri, HttpCode* responce_code, Error* err) const noexcept = 0;
    virtual ~HttpClient() = default;

};

Error HttpCodeToWorkerError(const HttpCode& code);

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

#endif //HIDRA2_HTTP_CLIENT_H
