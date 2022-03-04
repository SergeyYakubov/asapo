import * as grpcWeb from 'grpc-web';

import * as AsapoMonitoringCommonService_pb from './AsapoMonitoringCommonService_pb';
import * as AsapoMonitoringIngestService_pb from './AsapoMonitoringIngestService_pb';


export class AsapoMonitoringIngestServiceClient {
  constructor (hostname: string,
               credentials?: null | { [index: string]: string; },
               options?: null | { [index: string]: any; });

  insertReceiverDataPoints(
    request: AsapoMonitoringIngestService_pb.ReceiverDataPointContainer,
    metadata: grpcWeb.Metadata | undefined,
    callback: (err: grpcWeb.Error,
               response: AsapoMonitoringCommonService_pb.Empty) => void
  ): grpcWeb.ClientReadableStream<AsapoMonitoringCommonService_pb.Empty>;

  insertBrokerDataPoints(
    request: AsapoMonitoringIngestService_pb.BrokerDataPointContainer,
    metadata: grpcWeb.Metadata | undefined,
    callback: (err: grpcWeb.Error,
               response: AsapoMonitoringCommonService_pb.Empty) => void
  ): grpcWeb.ClientReadableStream<AsapoMonitoringCommonService_pb.Empty>;

  insertFtsDataPoints(
    request: AsapoMonitoringIngestService_pb.FtsToConsumerDataPointContainer,
    metadata: grpcWeb.Metadata | undefined,
    callback: (err: grpcWeb.Error,
               response: AsapoMonitoringCommonService_pb.Empty) => void
  ): grpcWeb.ClientReadableStream<AsapoMonitoringCommonService_pb.Empty>;

}

export class AsapoMonitoringIngestServicePromiseClient {
  constructor (hostname: string,
               credentials?: null | { [index: string]: string; },
               options?: null | { [index: string]: any; });

  insertReceiverDataPoints(
    request: AsapoMonitoringIngestService_pb.ReceiverDataPointContainer,
    metadata?: grpcWeb.Metadata
  ): Promise<AsapoMonitoringCommonService_pb.Empty>;

  insertBrokerDataPoints(
    request: AsapoMonitoringIngestService_pb.BrokerDataPointContainer,
    metadata?: grpcWeb.Metadata
  ): Promise<AsapoMonitoringCommonService_pb.Empty>;

  insertFtsDataPoints(
    request: AsapoMonitoringIngestService_pb.FtsToConsumerDataPointContainer,
    metadata?: grpcWeb.Metadata
  ): Promise<AsapoMonitoringCommonService_pb.Empty>;

}

