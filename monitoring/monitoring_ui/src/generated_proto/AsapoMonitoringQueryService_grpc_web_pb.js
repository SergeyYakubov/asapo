/**
 * @fileoverview gRPC-Web generated client stub for 
 * @enhanceable
 * @public
 */

// GENERATED CODE -- DO NOT EDIT!


/* eslint-disable */
// @ts-nocheck



const grpc = {};
grpc.web = require('grpc-web');


var AsapoMonitoringCommonService_pb = require('./AsapoMonitoringCommonService_pb.js')
const proto = require('./AsapoMonitoringQueryService_pb.js');

/**
 * @param {string} hostname
 * @param {?Object} credentials
 * @param {?Object} options
 * @constructor
 * @struct
 * @final
 */
proto.AsapoMonitoringQueryServiceClient =
    function(hostname, credentials, options) {
  if (!options) options = {};
  options['format'] = 'text';

  /**
   * @private @const {!grpc.web.GrpcWebClientBase} The client
   */
  this.client_ = new grpc.web.GrpcWebClientBase(options);

  /**
   * @private @const {string} The hostname
   */
  this.hostname_ = hostname;

};


/**
 * @param {string} hostname
 * @param {?Object} credentials
 * @param {?Object} options
 * @constructor
 * @struct
 * @final
 */
proto.AsapoMonitoringQueryServicePromiseClient =
    function(hostname, credentials, options) {
  if (!options) options = {};
  options['format'] = 'text';

  /**
   * @private @const {!grpc.web.GrpcWebClientBase} The client
   */
  this.client_ = new grpc.web.GrpcWebClientBase(options);

  /**
   * @private @const {string} The hostname
   */
  this.hostname_ = hostname;

};


/**
 * @const
 * @type {!grpc.web.MethodDescriptor<
 *   !proto.Empty,
 *   !proto.MetadataResponse>}
 */
const methodDescriptor_AsapoMonitoringQueryService_GetMetadata = new grpc.web.MethodDescriptor(
  '/AsapoMonitoringQueryService/GetMetadata',
  grpc.web.MethodType.UNARY,
  AsapoMonitoringCommonService_pb.Empty,
  proto.MetadataResponse,
  /**
   * @param {!proto.Empty} request
   * @return {!Uint8Array}
   */
  function(request) {
    return request.serializeBinary();
  },
  proto.MetadataResponse.deserializeBinary
);


/**
 * @const
 * @type {!grpc.web.AbstractClientBase.MethodInfo<
 *   !proto.Empty,
 *   !proto.MetadataResponse>}
 */
const methodInfo_AsapoMonitoringQueryService_GetMetadata = new grpc.web.AbstractClientBase.MethodInfo(
  proto.MetadataResponse,
  /**
   * @param {!proto.Empty} request
   * @return {!Uint8Array}
   */
  function(request) {
    return request.serializeBinary();
  },
  proto.MetadataResponse.deserializeBinary
);


/**
 * @param {!proto.Empty} request The
 *     request proto
 * @param {?Object<string, string>} metadata User defined
 *     call metadata
 * @param {function(?grpc.web.Error, ?proto.MetadataResponse)}
 *     callback The callback function(error, response)
 * @return {!grpc.web.ClientReadableStream<!proto.MetadataResponse>|undefined}
 *     The XHR Node Readable Stream
 */
proto.AsapoMonitoringQueryServiceClient.prototype.getMetadata =
    function(request, metadata, callback) {
  return this.client_.rpcCall(this.hostname_ +
      '/AsapoMonitoringQueryService/GetMetadata',
      request,
      metadata || {},
      methodDescriptor_AsapoMonitoringQueryService_GetMetadata,
      callback);
};


/**
 * @param {!proto.Empty} request The
 *     request proto
 * @param {?Object<string, string>} metadata User defined
 *     call metadata
 * @return {!Promise<!proto.MetadataResponse>}
 *     Promise that resolves to the response
 */
proto.AsapoMonitoringQueryServicePromiseClient.prototype.getMetadata =
    function(request, metadata) {
  return this.client_.unaryCall(this.hostname_ +
      '/AsapoMonitoringQueryService/GetMetadata',
      request,
      metadata || {},
      methodDescriptor_AsapoMonitoringQueryService_GetMetadata);
};


/**
 * @const
 * @type {!grpc.web.MethodDescriptor<
 *   !proto.ToplogyQuery,
 *   !proto.TopologyResponse>}
 */
const methodDescriptor_AsapoMonitoringQueryService_GetTopology = new grpc.web.MethodDescriptor(
  '/AsapoMonitoringQueryService/GetTopology',
  grpc.web.MethodType.UNARY,
  proto.ToplogyQuery,
  proto.TopologyResponse,
  /**
   * @param {!proto.ToplogyQuery} request
   * @return {!Uint8Array}
   */
  function(request) {
    return request.serializeBinary();
  },
  proto.TopologyResponse.deserializeBinary
);


/**
 * @const
 * @type {!grpc.web.AbstractClientBase.MethodInfo<
 *   !proto.ToplogyQuery,
 *   !proto.TopologyResponse>}
 */
const methodInfo_AsapoMonitoringQueryService_GetTopology = new grpc.web.AbstractClientBase.MethodInfo(
  proto.TopologyResponse,
  /**
   * @param {!proto.ToplogyQuery} request
   * @return {!Uint8Array}
   */
  function(request) {
    return request.serializeBinary();
  },
  proto.TopologyResponse.deserializeBinary
);


/**
 * @param {!proto.ToplogyQuery} request The
 *     request proto
 * @param {?Object<string, string>} metadata User defined
 *     call metadata
 * @param {function(?grpc.web.Error, ?proto.TopologyResponse)}
 *     callback The callback function(error, response)
 * @return {!grpc.web.ClientReadableStream<!proto.TopologyResponse>|undefined}
 *     The XHR Node Readable Stream
 */
proto.AsapoMonitoringQueryServiceClient.prototype.getTopology =
    function(request, metadata, callback) {
  return this.client_.rpcCall(this.hostname_ +
      '/AsapoMonitoringQueryService/GetTopology',
      request,
      metadata || {},
      methodDescriptor_AsapoMonitoringQueryService_GetTopology,
      callback);
};


/**
 * @param {!proto.ToplogyQuery} request The
 *     request proto
 * @param {?Object<string, string>} metadata User defined
 *     call metadata
 * @return {!Promise<!proto.TopologyResponse>}
 *     Promise that resolves to the response
 */
proto.AsapoMonitoringQueryServicePromiseClient.prototype.getTopology =
    function(request, metadata) {
  return this.client_.unaryCall(this.hostname_ +
      '/AsapoMonitoringQueryService/GetTopology',
      request,
      metadata || {},
      methodDescriptor_AsapoMonitoringQueryService_GetTopology);
};


/**
 * @const
 * @type {!grpc.web.MethodDescriptor<
 *   !proto.DataPointsQuery,
 *   !proto.DataPointsResponse>}
 */
const methodDescriptor_AsapoMonitoringQueryService_GetDataPoints = new grpc.web.MethodDescriptor(
  '/AsapoMonitoringQueryService/GetDataPoints',
  grpc.web.MethodType.UNARY,
  proto.DataPointsQuery,
  proto.DataPointsResponse,
  /**
   * @param {!proto.DataPointsQuery} request
   * @return {!Uint8Array}
   */
  function(request) {
    return request.serializeBinary();
  },
  proto.DataPointsResponse.deserializeBinary
);


/**
 * @const
 * @type {!grpc.web.AbstractClientBase.MethodInfo<
 *   !proto.DataPointsQuery,
 *   !proto.DataPointsResponse>}
 */
const methodInfo_AsapoMonitoringQueryService_GetDataPoints = new grpc.web.AbstractClientBase.MethodInfo(
  proto.DataPointsResponse,
  /**
   * @param {!proto.DataPointsQuery} request
   * @return {!Uint8Array}
   */
  function(request) {
    return request.serializeBinary();
  },
  proto.DataPointsResponse.deserializeBinary
);


/**
 * @param {!proto.DataPointsQuery} request The
 *     request proto
 * @param {?Object<string, string>} metadata User defined
 *     call metadata
 * @param {function(?grpc.web.Error, ?proto.DataPointsResponse)}
 *     callback The callback function(error, response)
 * @return {!grpc.web.ClientReadableStream<!proto.DataPointsResponse>|undefined}
 *     The XHR Node Readable Stream
 */
proto.AsapoMonitoringQueryServiceClient.prototype.getDataPoints =
    function(request, metadata, callback) {
  return this.client_.rpcCall(this.hostname_ +
      '/AsapoMonitoringQueryService/GetDataPoints',
      request,
      metadata || {},
      methodDescriptor_AsapoMonitoringQueryService_GetDataPoints,
      callback);
};


/**
 * @param {!proto.DataPointsQuery} request The
 *     request proto
 * @param {?Object<string, string>} metadata User defined
 *     call metadata
 * @return {!Promise<!proto.DataPointsResponse>}
 *     Promise that resolves to the response
 */
proto.AsapoMonitoringQueryServicePromiseClient.prototype.getDataPoints =
    function(request, metadata) {
  return this.client_.unaryCall(this.hostname_ +
      '/AsapoMonitoringQueryService/GetDataPoints',
      request,
      metadata || {},
      methodDescriptor_AsapoMonitoringQueryService_GetDataPoints);
};


module.exports = proto;

