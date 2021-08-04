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
const proto = require('./AsapoMonitoringIngestService_pb.js');

/**
 * @param {string} hostname
 * @param {?Object} credentials
 * @param {?Object} options
 * @constructor
 * @struct
 * @final
 */
proto.AsapoMonitoringIngestServiceClient =
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
proto.AsapoMonitoringIngestServicePromiseClient =
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
 *   !proto.ReceiverDataPointContainer,
 *   !proto.Empty>}
 */
const methodDescriptor_AsapoMonitoringIngestService_InsertReceiverDataPoints = new grpc.web.MethodDescriptor(
  '/AsapoMonitoringIngestService/InsertReceiverDataPoints',
  grpc.web.MethodType.UNARY,
  proto.ReceiverDataPointContainer,
  AsapoMonitoringCommonService_pb.Empty,
  /**
   * @param {!proto.ReceiverDataPointContainer} request
   * @return {!Uint8Array}
   */
  function(request) {
    return request.serializeBinary();
  },
  AsapoMonitoringCommonService_pb.Empty.deserializeBinary
);


/**
 * @const
 * @type {!grpc.web.AbstractClientBase.MethodInfo<
 *   !proto.ReceiverDataPointContainer,
 *   !proto.Empty>}
 */
const methodInfo_AsapoMonitoringIngestService_InsertReceiverDataPoints = new grpc.web.AbstractClientBase.MethodInfo(
  AsapoMonitoringCommonService_pb.Empty,
  /**
   * @param {!proto.ReceiverDataPointContainer} request
   * @return {!Uint8Array}
   */
  function(request) {
    return request.serializeBinary();
  },
  AsapoMonitoringCommonService_pb.Empty.deserializeBinary
);


/**
 * @param {!proto.ReceiverDataPointContainer} request The
 *     request proto
 * @param {?Object<string, string>} metadata User defined
 *     call metadata
 * @param {function(?grpc.web.Error, ?proto.Empty)}
 *     callback The callback function(error, response)
 * @return {!grpc.web.ClientReadableStream<!proto.Empty>|undefined}
 *     The XHR Node Readable Stream
 */
proto.AsapoMonitoringIngestServiceClient.prototype.insertReceiverDataPoints =
    function(request, metadata, callback) {
  return this.client_.rpcCall(this.hostname_ +
      '/AsapoMonitoringIngestService/InsertReceiverDataPoints',
      request,
      metadata || {},
      methodDescriptor_AsapoMonitoringIngestService_InsertReceiverDataPoints,
      callback);
};


/**
 * @param {!proto.ReceiverDataPointContainer} request The
 *     request proto
 * @param {?Object<string, string>} metadata User defined
 *     call metadata
 * @return {!Promise<!proto.Empty>}
 *     Promise that resolves to the response
 */
proto.AsapoMonitoringIngestServicePromiseClient.prototype.insertReceiverDataPoints =
    function(request, metadata) {
  return this.client_.unaryCall(this.hostname_ +
      '/AsapoMonitoringIngestService/InsertReceiverDataPoints',
      request,
      metadata || {},
      methodDescriptor_AsapoMonitoringIngestService_InsertReceiverDataPoints);
};


/**
 * @const
 * @type {!grpc.web.MethodDescriptor<
 *   !proto.BrokerDataPointContainer,
 *   !proto.Empty>}
 */
const methodDescriptor_AsapoMonitoringIngestService_InsertBrokerDataPoints = new grpc.web.MethodDescriptor(
  '/AsapoMonitoringIngestService/InsertBrokerDataPoints',
  grpc.web.MethodType.UNARY,
  proto.BrokerDataPointContainer,
  AsapoMonitoringCommonService_pb.Empty,
  /**
   * @param {!proto.BrokerDataPointContainer} request
   * @return {!Uint8Array}
   */
  function(request) {
    return request.serializeBinary();
  },
  AsapoMonitoringCommonService_pb.Empty.deserializeBinary
);


/**
 * @const
 * @type {!grpc.web.AbstractClientBase.MethodInfo<
 *   !proto.BrokerDataPointContainer,
 *   !proto.Empty>}
 */
const methodInfo_AsapoMonitoringIngestService_InsertBrokerDataPoints = new grpc.web.AbstractClientBase.MethodInfo(
  AsapoMonitoringCommonService_pb.Empty,
  /**
   * @param {!proto.BrokerDataPointContainer} request
   * @return {!Uint8Array}
   */
  function(request) {
    return request.serializeBinary();
  },
  AsapoMonitoringCommonService_pb.Empty.deserializeBinary
);


/**
 * @param {!proto.BrokerDataPointContainer} request The
 *     request proto
 * @param {?Object<string, string>} metadata User defined
 *     call metadata
 * @param {function(?grpc.web.Error, ?proto.Empty)}
 *     callback The callback function(error, response)
 * @return {!grpc.web.ClientReadableStream<!proto.Empty>|undefined}
 *     The XHR Node Readable Stream
 */
proto.AsapoMonitoringIngestServiceClient.prototype.insertBrokerDataPoints =
    function(request, metadata, callback) {
  return this.client_.rpcCall(this.hostname_ +
      '/AsapoMonitoringIngestService/InsertBrokerDataPoints',
      request,
      metadata || {},
      methodDescriptor_AsapoMonitoringIngestService_InsertBrokerDataPoints,
      callback);
};


/**
 * @param {!proto.BrokerDataPointContainer} request The
 *     request proto
 * @param {?Object<string, string>} metadata User defined
 *     call metadata
 * @return {!Promise<!proto.Empty>}
 *     Promise that resolves to the response
 */
proto.AsapoMonitoringIngestServicePromiseClient.prototype.insertBrokerDataPoints =
    function(request, metadata) {
  return this.client_.unaryCall(this.hostname_ +
      '/AsapoMonitoringIngestService/InsertBrokerDataPoints',
      request,
      metadata || {},
      methodDescriptor_AsapoMonitoringIngestService_InsertBrokerDataPoints);
};


/**
 * @const
 * @type {!grpc.web.MethodDescriptor<
 *   !proto.FtsToConsumerDataPointContainer,
 *   !proto.Empty>}
 */
const methodDescriptor_AsapoMonitoringIngestService_InsertFtsDataPoints = new grpc.web.MethodDescriptor(
  '/AsapoMonitoringIngestService/InsertFtsDataPoints',
  grpc.web.MethodType.UNARY,
  proto.FtsToConsumerDataPointContainer,
  AsapoMonitoringCommonService_pb.Empty,
  /**
   * @param {!proto.FtsToConsumerDataPointContainer} request
   * @return {!Uint8Array}
   */
  function(request) {
    return request.serializeBinary();
  },
  AsapoMonitoringCommonService_pb.Empty.deserializeBinary
);


/**
 * @const
 * @type {!grpc.web.AbstractClientBase.MethodInfo<
 *   !proto.FtsToConsumerDataPointContainer,
 *   !proto.Empty>}
 */
const methodInfo_AsapoMonitoringIngestService_InsertFtsDataPoints = new grpc.web.AbstractClientBase.MethodInfo(
  AsapoMonitoringCommonService_pb.Empty,
  /**
   * @param {!proto.FtsToConsumerDataPointContainer} request
   * @return {!Uint8Array}
   */
  function(request) {
    return request.serializeBinary();
  },
  AsapoMonitoringCommonService_pb.Empty.deserializeBinary
);


/**
 * @param {!proto.FtsToConsumerDataPointContainer} request The
 *     request proto
 * @param {?Object<string, string>} metadata User defined
 *     call metadata
 * @param {function(?grpc.web.Error, ?proto.Empty)}
 *     callback The callback function(error, response)
 * @return {!grpc.web.ClientReadableStream<!proto.Empty>|undefined}
 *     The XHR Node Readable Stream
 */
proto.AsapoMonitoringIngestServiceClient.prototype.insertFtsDataPoints =
    function(request, metadata, callback) {
  return this.client_.rpcCall(this.hostname_ +
      '/AsapoMonitoringIngestService/InsertFtsDataPoints',
      request,
      metadata || {},
      methodDescriptor_AsapoMonitoringIngestService_InsertFtsDataPoints,
      callback);
};


/**
 * @param {!proto.FtsToConsumerDataPointContainer} request The
 *     request proto
 * @param {?Object<string, string>} metadata User defined
 *     call metadata
 * @return {!Promise<!proto.Empty>}
 *     Promise that resolves to the response
 */
proto.AsapoMonitoringIngestServicePromiseClient.prototype.insertFtsDataPoints =
    function(request, metadata) {
  return this.client_.unaryCall(this.hostname_ +
      '/AsapoMonitoringIngestService/InsertFtsDataPoints',
      request,
      metadata || {},
      methodDescriptor_AsapoMonitoringIngestService_InsertFtsDataPoints);
};


module.exports = proto;

