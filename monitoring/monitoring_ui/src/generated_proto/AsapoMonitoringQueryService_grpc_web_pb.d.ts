import * as grpcWeb from 'grpc-web';

import * as AsapoMonitoringCommonService_pb from './AsapoMonitoringCommonService_pb';
import * as AsapoMonitoringQueryService_pb from './AsapoMonitoringQueryService_pb';


export class AsapoMonitoringQueryServiceClient {
  constructor (hostname: string,
               credentials?: null | { [index: string]: string; },
               options?: null | { [index: string]: any; });

  getMetadata(
    request: AsapoMonitoringCommonService_pb.Empty,
    metadata: grpcWeb.Metadata | undefined,
    callback: (err: grpcWeb.Error,
               response: AsapoMonitoringQueryService_pb.MetadataResponse) => void
  ): grpcWeb.ClientReadableStream<AsapoMonitoringQueryService_pb.MetadataResponse>;

  getTopology(
    request: AsapoMonitoringQueryService_pb.ToplogyQuery,
    metadata: grpcWeb.Metadata | undefined,
    callback: (err: grpcWeb.Error,
               response: AsapoMonitoringQueryService_pb.TopologyResponse) => void
  ): grpcWeb.ClientReadableStream<AsapoMonitoringQueryService_pb.TopologyResponse>;

  getDataPoints(
    request: AsapoMonitoringQueryService_pb.DataPointsQuery,
    metadata: grpcWeb.Metadata | undefined,
    callback: (err: grpcWeb.Error,
               response: AsapoMonitoringQueryService_pb.DataPointsResponse) => void
  ): grpcWeb.ClientReadableStream<AsapoMonitoringQueryService_pb.DataPointsResponse>;

}

export class AsapoMonitoringQueryServicePromiseClient {
  constructor (hostname: string,
               credentials?: null | { [index: string]: string; },
               options?: null | { [index: string]: any; });

  getMetadata(
    request: AsapoMonitoringCommonService_pb.Empty,
    metadata?: grpcWeb.Metadata
  ): Promise<AsapoMonitoringQueryService_pb.MetadataResponse>;

  getTopology(
    request: AsapoMonitoringQueryService_pb.ToplogyQuery,
    metadata?: grpcWeb.Metadata
  ): Promise<AsapoMonitoringQueryService_pb.TopologyResponse>;

  getDataPoints(
    request: AsapoMonitoringQueryService_pb.DataPointsQuery,
    metadata?: grpcWeb.Metadata
  ): Promise<AsapoMonitoringQueryService_pb.DataPointsResponse>;

}

