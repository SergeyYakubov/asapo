import * as jspb from 'google-protobuf'

import * as AsapoMonitoringCommonService_pb from './AsapoMonitoringCommonService_pb';


export class DataPointsQuery extends jspb.Message {
  getFromtimestamp(): number;
  setFromtimestamp(value: number): DataPointsQuery;

  getTotimestamp(): number;
  setTotimestamp(value: number): DataPointsQuery;

  getBeamtimefilter(): string;
  setBeamtimefilter(value: string): DataPointsQuery;

  getSourcefilter(): string;
  setSourcefilter(value: string): DataPointsQuery;

  getStreamfilter(): string;
  setStreamfilter(value: string): DataPointsQuery;

  getReceiverfilter(): string;
  setReceiverfilter(value: string): DataPointsQuery;

  getFrompipelinestepfilter(): string;
  setFrompipelinestepfilter(value: string): DataPointsQuery;

  getTopipelinestepfilter(): string;
  setTopipelinestepfilter(value: string): DataPointsQuery;

  getPipelinequerymode(): ipelineConnectionQueryMode;
  setPipelinequerymode(value: ipelineConnectionQueryMode): DataPointsQuery;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): DataPointsQuery.AsObject;
  static toObject(includeInstance: boolean, msg: DataPointsQuery): DataPointsQuery.AsObject;
  static serializeBinaryToWriter(message: DataPointsQuery, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): DataPointsQuery;
  static deserializeBinaryFromReader(message: DataPointsQuery, reader: jspb.BinaryReader): DataPointsQuery;
}

export namespace DataPointsQuery {
  export type AsObject = {
    fromtimestamp: number,
    totimestamp: number,
    beamtimefilter: string,
    sourcefilter: string,
    streamfilter: string,
    receiverfilter: string,
    frompipelinestepfilter: string,
    topipelinestepfilter: string,
    pipelinequerymode: ipelineConnectionQueryMode,
  }
}

export class TotalTransferRateDataPoint extends jspb.Message {
  getTotalbytespersecrecv(): number;
  setTotalbytespersecrecv(value: number): TotalTransferRateDataPoint;

  getTotalbytespersecsend(): number;
  setTotalbytespersecsend(value: number): TotalTransferRateDataPoint;

  getTotalbytespersecrdssend(): number;
  setTotalbytespersecrdssend(value: number): TotalTransferRateDataPoint;

  getTotalbytespersecftssend(): number;
  setTotalbytespersecftssend(value: number): TotalTransferRateDataPoint;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): TotalTransferRateDataPoint.AsObject;
  static toObject(includeInstance: boolean, msg: TotalTransferRateDataPoint): TotalTransferRateDataPoint.AsObject;
  static serializeBinaryToWriter(message: TotalTransferRateDataPoint, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): TotalTransferRateDataPoint;
  static deserializeBinaryFromReader(message: TotalTransferRateDataPoint, reader: jspb.BinaryReader): TotalTransferRateDataPoint;
}

export namespace TotalTransferRateDataPoint {
  export type AsObject = {
    totalbytespersecrecv: number,
    totalbytespersecsend: number,
    totalbytespersecrdssend: number,
    totalbytespersecftssend: number,
  }
}

export class TotalFileRateDataPoint extends jspb.Message {
  getTotalrequests(): number;
  setTotalrequests(value: number): TotalFileRateDataPoint;

  getCachemisses(): number;
  setCachemisses(value: number): TotalFileRateDataPoint;

  getFromcache(): number;
  setFromcache(value: number): TotalFileRateDataPoint;

  getFromdisk(): number;
  setFromdisk(value: number): TotalFileRateDataPoint;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): TotalFileRateDataPoint.AsObject;
  static toObject(includeInstance: boolean, msg: TotalFileRateDataPoint): TotalFileRateDataPoint.AsObject;
  static serializeBinaryToWriter(message: TotalFileRateDataPoint, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): TotalFileRateDataPoint;
  static deserializeBinaryFromReader(message: TotalFileRateDataPoint, reader: jspb.BinaryReader): TotalFileRateDataPoint;
}

export namespace TotalFileRateDataPoint {
  export type AsObject = {
    totalrequests: number,
    cachemisses: number,
    fromcache: number,
    fromdisk: number,
  }
}

export class TaskTimeDataPoint extends jspb.Message {
  getReceiveiotimeus(): number;
  setReceiveiotimeus(value: number): TaskTimeDataPoint;

  getWritetodisktimeus(): number;
  setWritetodisktimeus(value: number): TaskTimeDataPoint;

  getWritetodatabasetimeus(): number;
  setWritetodatabasetimeus(value: number): TaskTimeDataPoint;

  getRdssendtoconsumertimeus(): number;
  setRdssendtoconsumertimeus(value: number): TaskTimeDataPoint;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): TaskTimeDataPoint.AsObject;
  static toObject(includeInstance: boolean, msg: TaskTimeDataPoint): TaskTimeDataPoint.AsObject;
  static serializeBinaryToWriter(message: TaskTimeDataPoint, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): TaskTimeDataPoint;
  static deserializeBinaryFromReader(message: TaskTimeDataPoint, reader: jspb.BinaryReader): TaskTimeDataPoint;
}

export namespace TaskTimeDataPoint {
  export type AsObject = {
    receiveiotimeus: number,
    writetodisktimeus: number,
    writetodatabasetimeus: number,
    rdssendtoconsumertimeus: number,
  }
}

export class RdsMemoryUsageDataPoint extends jspb.Message {
  getTotalusedmemory(): number;
  setTotalusedmemory(value: number): RdsMemoryUsageDataPoint;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): RdsMemoryUsageDataPoint.AsObject;
  static toObject(includeInstance: boolean, msg: RdsMemoryUsageDataPoint): RdsMemoryUsageDataPoint.AsObject;
  static serializeBinaryToWriter(message: RdsMemoryUsageDataPoint, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): RdsMemoryUsageDataPoint;
  static deserializeBinaryFromReader(message: RdsMemoryUsageDataPoint, reader: jspb.BinaryReader): RdsMemoryUsageDataPoint;
}

export namespace RdsMemoryUsageDataPoint {
  export type AsObject = {
    totalusedmemory: number,
  }
}

export class DataPointsResponse extends jspb.Message {
  getStarttimestampinsec(): number;
  setStarttimestampinsec(value: number): DataPointsResponse;

  getTimeintervalinsec(): number;
  setTimeintervalinsec(value: number): DataPointsResponse;

  getTransferratesList(): Array<TotalTransferRateDataPoint>;
  setTransferratesList(value: Array<TotalTransferRateDataPoint>): DataPointsResponse;
  clearTransferratesList(): DataPointsResponse;
  addTransferrates(value?: TotalTransferRateDataPoint, index?: number): TotalTransferRateDataPoint;

  getFileratesList(): Array<TotalFileRateDataPoint>;
  setFileratesList(value: Array<TotalFileRateDataPoint>): DataPointsResponse;
  clearFileratesList(): DataPointsResponse;
  addFilerates(value?: TotalFileRateDataPoint, index?: number): TotalFileRateDataPoint;

  getTasktimesList(): Array<TaskTimeDataPoint>;
  setTasktimesList(value: Array<TaskTimeDataPoint>): DataPointsResponse;
  clearTasktimesList(): DataPointsResponse;
  addTasktimes(value?: TaskTimeDataPoint, index?: number): TaskTimeDataPoint;

  getMemoryusagesList(): Array<RdsMemoryUsageDataPoint>;
  setMemoryusagesList(value: Array<RdsMemoryUsageDataPoint>): DataPointsResponse;
  clearMemoryusagesList(): DataPointsResponse;
  addMemoryusages(value?: RdsMemoryUsageDataPoint, index?: number): RdsMemoryUsageDataPoint;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): DataPointsResponse.AsObject;
  static toObject(includeInstance: boolean, msg: DataPointsResponse): DataPointsResponse.AsObject;
  static serializeBinaryToWriter(message: DataPointsResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): DataPointsResponse;
  static deserializeBinaryFromReader(message: DataPointsResponse, reader: jspb.BinaryReader): DataPointsResponse;
}

export namespace DataPointsResponse {
  export type AsObject = {
    starttimestampinsec: number,
    timeintervalinsec: number,
    transferratesList: Array<TotalTransferRateDataPoint.AsObject>,
    fileratesList: Array<TotalFileRateDataPoint.AsObject>,
    tasktimesList: Array<TaskTimeDataPoint.AsObject>,
    memoryusagesList: Array<RdsMemoryUsageDataPoint.AsObject>,
  }
}

export class MetadataResponse extends jspb.Message {
  getClustername(): string;
  setClustername(value: string): MetadataResponse;

  getBeamtimeList(): Array<string>;
  setBeamtimeList(value: Array<string>): MetadataResponse;
  clearBeamtimeList(): MetadataResponse;
  addBeamtime(value: string, index?: number): MetadataResponse;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): MetadataResponse.AsObject;
  static toObject(includeInstance: boolean, msg: MetadataResponse): MetadataResponse.AsObject;
  static serializeBinaryToWriter(message: MetadataResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): MetadataResponse;
  static deserializeBinaryFromReader(message: MetadataResponse, reader: jspb.BinaryReader): MetadataResponse;
}

export namespace MetadataResponse {
  export type AsObject = {
    clustername: string,
    beamtimeList: Array<string>,
  }
}

export class ToplogyQuery extends jspb.Message {
  getFromtimestamp(): number;
  setFromtimestamp(value: number): ToplogyQuery;

  getTotimestamp(): number;
  setTotimestamp(value: number): ToplogyQuery;

  getBeamtimefilter(): string;
  setBeamtimefilter(value: string): ToplogyQuery;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): ToplogyQuery.AsObject;
  static toObject(includeInstance: boolean, msg: ToplogyQuery): ToplogyQuery.AsObject;
  static serializeBinaryToWriter(message: ToplogyQuery, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): ToplogyQuery;
  static deserializeBinaryFromReader(message: ToplogyQuery, reader: jspb.BinaryReader): ToplogyQuery;
}

export namespace ToplogyQuery {
  export type AsObject = {
    fromtimestamp: number,
    totimestamp: number,
    beamtimefilter: string,
  }
}

export class TopologyResponseNode extends jspb.Message {
  getNodeid(): string;
  setNodeid(value: string): TopologyResponseNode;

  getLevel(): number;
  setLevel(value: number): TopologyResponseNode;

  getConsumerinstancesList(): Array<string>;
  setConsumerinstancesList(value: Array<string>): TopologyResponseNode;
  clearConsumerinstancesList(): TopologyResponseNode;
  addConsumerinstances(value: string, index?: number): TopologyResponseNode;

  getProducerinstancesList(): Array<string>;
  setProducerinstancesList(value: Array<string>): TopologyResponseNode;
  clearProducerinstancesList(): TopologyResponseNode;
  addProducerinstances(value: string, index?: number): TopologyResponseNode;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): TopologyResponseNode.AsObject;
  static toObject(includeInstance: boolean, msg: TopologyResponseNode): TopologyResponseNode.AsObject;
  static serializeBinaryToWriter(message: TopologyResponseNode, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): TopologyResponseNode;
  static deserializeBinaryFromReader(message: TopologyResponseNode, reader: jspb.BinaryReader): TopologyResponseNode;
}

export namespace TopologyResponseNode {
  export type AsObject = {
    nodeid: string,
    level: number,
    consumerinstancesList: Array<string>,
    producerinstancesList: Array<string>,
  }
}

export class TopologyResponseEdge extends jspb.Message {
  getFromnodeid(): string;
  setFromnodeid(value: string): TopologyResponseEdge;

  getTonodeid(): string;
  setTonodeid(value: string): TopologyResponseEdge;

  getSourcename(): string;
  setSourcename(value: string): TopologyResponseEdge;

  getInvolvedreceiversList(): Array<string>;
  setInvolvedreceiversList(value: Array<string>): TopologyResponseEdge;
  clearInvolvedreceiversList(): TopologyResponseEdge;
  addInvolvedreceivers(value: string, index?: number): TopologyResponseEdge;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): TopologyResponseEdge.AsObject;
  static toObject(includeInstance: boolean, msg: TopologyResponseEdge): TopologyResponseEdge.AsObject;
  static serializeBinaryToWriter(message: TopologyResponseEdge, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): TopologyResponseEdge;
  static deserializeBinaryFromReader(message: TopologyResponseEdge, reader: jspb.BinaryReader): TopologyResponseEdge;
}

export namespace TopologyResponseEdge {
  export type AsObject = {
    fromnodeid: string,
    tonodeid: string,
    sourcename: string,
    involvedreceiversList: Array<string>,
  }
}

export class TopologyResponse extends jspb.Message {
  getNodesList(): Array<TopologyResponseNode>;
  setNodesList(value: Array<TopologyResponseNode>): TopologyResponse;
  clearNodesList(): TopologyResponse;
  addNodes(value?: TopologyResponseNode, index?: number): TopologyResponseNode;

  getEdgesList(): Array<TopologyResponseEdge>;
  setEdgesList(value: Array<TopologyResponseEdge>): TopologyResponse;
  clearEdgesList(): TopologyResponse;
  addEdges(value?: TopologyResponseEdge, index?: number): TopologyResponseEdge;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): TopologyResponse.AsObject;
  static toObject(includeInstance: boolean, msg: TopologyResponse): TopologyResponse.AsObject;
  static serializeBinaryToWriter(message: TopologyResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): TopologyResponse;
  static deserializeBinaryFromReader(message: TopologyResponse, reader: jspb.BinaryReader): TopologyResponse;
}

export namespace TopologyResponse {
  export type AsObject = {
    nodesList: Array<TopologyResponseNode.AsObject>,
    edgesList: Array<TopologyResponseEdge.AsObject>,
  }
}

export enum PipelineConnectionQueryMode { 
  INVALID_QUERY_MODE = 0,
  EXACTPATH = 1,
  JUSTRELATEDTOSTEP = 2,
}
