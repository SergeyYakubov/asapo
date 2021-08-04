import * as jspb from 'google-protobuf'

import * as AsapoMonitoringCommonService_pb from './AsapoMonitoringCommonService_pb';


export class ProducerToReceiverTransferDataPoint extends jspb.Message {
  getPipelinestepid(): string;
  setPipelinestepid(value: string): ProducerToReceiverTransferDataPoint;

  getProducerinstanceid(): string;
  setProducerinstanceid(value: string): ProducerToReceiverTransferDataPoint;

  getBeamtime(): string;
  setBeamtime(value: string): ProducerToReceiverTransferDataPoint;

  getSource(): string;
  setSource(value: string): ProducerToReceiverTransferDataPoint;

  getStream(): string;
  setStream(value: string): ProducerToReceiverTransferDataPoint;

  getFilecount(): number;
  setFilecount(value: number): ProducerToReceiverTransferDataPoint;

  getTotalfilesize(): number;
  setTotalfilesize(value: number): ProducerToReceiverTransferDataPoint;

  getTotaltransferreceivetimeinmicroseconds(): number;
  setTotaltransferreceivetimeinmicroseconds(value: number): ProducerToReceiverTransferDataPoint;

  getTotalwriteiotimeinmicroseconds(): number;
  setTotalwriteiotimeinmicroseconds(value: number): ProducerToReceiverTransferDataPoint;

  getTotaldbtimeinmicroseconds(): number;
  setTotaldbtimeinmicroseconds(value: number): ProducerToReceiverTransferDataPoint;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): ProducerToReceiverTransferDataPoint.AsObject;
  static toObject(includeInstance: boolean, msg: ProducerToReceiverTransferDataPoint): ProducerToReceiverTransferDataPoint.AsObject;
  static serializeBinaryToWriter(message: ProducerToReceiverTransferDataPoint, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): ProducerToReceiverTransferDataPoint;
  static deserializeBinaryFromReader(message: ProducerToReceiverTransferDataPoint, reader: jspb.BinaryReader): ProducerToReceiverTransferDataPoint;
}

export namespace ProducerToReceiverTransferDataPoint {
  export type AsObject = {
    pipelinestepid: string,
    producerinstanceid: string,
    beamtime: string,
    source: string,
    stream: string,
    filecount: number,
    totalfilesize: number,
    totaltransferreceivetimeinmicroseconds: number,
    totalwriteiotimeinmicroseconds: number,
    totaldbtimeinmicroseconds: number,
  }
}

export class BrokerRequestDataPoint extends jspb.Message {
  getPipelinestepid(): string;
  setPipelinestepid(value: string): BrokerRequestDataPoint;

  getConsumerinstanceid(): string;
  setConsumerinstanceid(value: string): BrokerRequestDataPoint;

  getCommand(): string;
  setCommand(value: string): BrokerRequestDataPoint;

  getBeamtime(): string;
  setBeamtime(value: string): BrokerRequestDataPoint;

  getSource(): string;
  setSource(value: string): BrokerRequestDataPoint;

  getStream(): string;
  setStream(value: string): BrokerRequestDataPoint;

  getFilecount(): number;
  setFilecount(value: number): BrokerRequestDataPoint;

  getTotalfilesize(): number;
  setTotalfilesize(value: number): BrokerRequestDataPoint;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): BrokerRequestDataPoint.AsObject;
  static toObject(includeInstance: boolean, msg: BrokerRequestDataPoint): BrokerRequestDataPoint.AsObject;
  static serializeBinaryToWriter(message: BrokerRequestDataPoint, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): BrokerRequestDataPoint;
  static deserializeBinaryFromReader(message: BrokerRequestDataPoint, reader: jspb.BinaryReader): BrokerRequestDataPoint;
}

export namespace BrokerRequestDataPoint {
  export type AsObject = {
    pipelinestepid: string,
    consumerinstanceid: string,
    command: string,
    beamtime: string,
    source: string,
    stream: string,
    filecount: number,
    totalfilesize: number,
  }
}

export class RdsMemoryDataPoint extends jspb.Message {
  getBeamtime(): string;
  setBeamtime(value: string): RdsMemoryDataPoint;

  getSource(): string;
  setSource(value: string): RdsMemoryDataPoint;

  getStream(): string;
  setStream(value: string): RdsMemoryDataPoint;

  getUsedbytes(): number;
  setUsedbytes(value: number): RdsMemoryDataPoint;

  getTotalbytes(): number;
  setTotalbytes(value: number): RdsMemoryDataPoint;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): RdsMemoryDataPoint.AsObject;
  static toObject(includeInstance: boolean, msg: RdsMemoryDataPoint): RdsMemoryDataPoint.AsObject;
  static serializeBinaryToWriter(message: RdsMemoryDataPoint, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): RdsMemoryDataPoint;
  static deserializeBinaryFromReader(message: RdsMemoryDataPoint, reader: jspb.BinaryReader): RdsMemoryDataPoint;
}

export namespace RdsMemoryDataPoint {
  export type AsObject = {
    beamtime: string,
    source: string,
    stream: string,
    usedbytes: number,
    totalbytes: number,
  }
}

export class RdsToConsumerDataPoint extends jspb.Message {
  getPipelinestepid(): string;
  setPipelinestepid(value: string): RdsToConsumerDataPoint;

  getConsumerinstanceid(): string;
  setConsumerinstanceid(value: string): RdsToConsumerDataPoint;

  getBeamtime(): string;
  setBeamtime(value: string): RdsToConsumerDataPoint;

  getSource(): string;
  setSource(value: string): RdsToConsumerDataPoint;

  getStream(): string;
  setStream(value: string): RdsToConsumerDataPoint;

  getHits(): number;
  setHits(value: number): RdsToConsumerDataPoint;

  getMisses(): number;
  setMisses(value: number): RdsToConsumerDataPoint;

  getTotalfilesize(): number;
  setTotalfilesize(value: number): RdsToConsumerDataPoint;

  getTotaltransfersendtimeinmicroseconds(): number;
  setTotaltransfersendtimeinmicroseconds(value: number): RdsToConsumerDataPoint;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): RdsToConsumerDataPoint.AsObject;
  static toObject(includeInstance: boolean, msg: RdsToConsumerDataPoint): RdsToConsumerDataPoint.AsObject;
  static serializeBinaryToWriter(message: RdsToConsumerDataPoint, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): RdsToConsumerDataPoint;
  static deserializeBinaryFromReader(message: RdsToConsumerDataPoint, reader: jspb.BinaryReader): RdsToConsumerDataPoint;
}

export namespace RdsToConsumerDataPoint {
  export type AsObject = {
    pipelinestepid: string,
    consumerinstanceid: string,
    beamtime: string,
    source: string,
    stream: string,
    hits: number,
    misses: number,
    totalfilesize: number,
    totaltransfersendtimeinmicroseconds: number,
  }
}

export class FdsToConsumerDataPoint extends jspb.Message {
  getPipelinestepid(): string;
  setPipelinestepid(value: string): FdsToConsumerDataPoint;

  getConsumerinstanceid(): string;
  setConsumerinstanceid(value: string): FdsToConsumerDataPoint;

  getBeamtime(): string;
  setBeamtime(value: string): FdsToConsumerDataPoint;

  getSource(): string;
  setSource(value: string): FdsToConsumerDataPoint;

  getStream(): string;
  setStream(value: string): FdsToConsumerDataPoint;

  getFilecount(): number;
  setFilecount(value: number): FdsToConsumerDataPoint;

  getTotalfilesize(): number;
  setTotalfilesize(value: number): FdsToConsumerDataPoint;

  getTotaltransfersendtimeinmicroseconds(): number;
  setTotaltransfersendtimeinmicroseconds(value: number): FdsToConsumerDataPoint;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): FdsToConsumerDataPoint.AsObject;
  static toObject(includeInstance: boolean, msg: FdsToConsumerDataPoint): FdsToConsumerDataPoint.AsObject;
  static serializeBinaryToWriter(message: FdsToConsumerDataPoint, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): FdsToConsumerDataPoint;
  static deserializeBinaryFromReader(message: FdsToConsumerDataPoint, reader: jspb.BinaryReader): FdsToConsumerDataPoint;
}

export namespace FdsToConsumerDataPoint {
  export type AsObject = {
    pipelinestepid: string,
    consumerinstanceid: string,
    beamtime: string,
    source: string,
    stream: string,
    filecount: number,
    totalfilesize: number,
    totaltransfersendtimeinmicroseconds: number,
  }
}

export class ReceiverDataPointContainer extends jspb.Message {
  getReceivername(): string;
  setReceivername(value: string): ReceiverDataPointContainer;

  getTimestampms(): number;
  setTimestampms(value: number): ReceiverDataPointContainer;

  getGroupedp2rtransfersList(): Array<ProducerToReceiverTransferDataPoint>;
  setGroupedp2rtransfersList(value: Array<ProducerToReceiverTransferDataPoint>): ReceiverDataPointContainer;
  clearGroupedp2rtransfersList(): ReceiverDataPointContainer;
  addGroupedp2rtransfers(value?: ProducerToReceiverTransferDataPoint, index?: number): ProducerToReceiverTransferDataPoint;

  getGroupedrds2ctransfersList(): Array<RdsToConsumerDataPoint>;
  setGroupedrds2ctransfersList(value: Array<RdsToConsumerDataPoint>): ReceiverDataPointContainer;
  clearGroupedrds2ctransfersList(): ReceiverDataPointContainer;
  addGroupedrds2ctransfers(value?: RdsToConsumerDataPoint, index?: number): RdsToConsumerDataPoint;

  getGroupedmemorystatsList(): Array<RdsMemoryDataPoint>;
  setGroupedmemorystatsList(value: Array<RdsMemoryDataPoint>): ReceiverDataPointContainer;
  clearGroupedmemorystatsList(): ReceiverDataPointContainer;
  addGroupedmemorystats(value?: RdsMemoryDataPoint, index?: number): RdsMemoryDataPoint;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): ReceiverDataPointContainer.AsObject;
  static toObject(includeInstance: boolean, msg: ReceiverDataPointContainer): ReceiverDataPointContainer.AsObject;
  static serializeBinaryToWriter(message: ReceiverDataPointContainer, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): ReceiverDataPointContainer;
  static deserializeBinaryFromReader(message: ReceiverDataPointContainer, reader: jspb.BinaryReader): ReceiverDataPointContainer;
}

export namespace ReceiverDataPointContainer {
  export type AsObject = {
    receivername: string,
    timestampms: number,
    groupedp2rtransfersList: Array<ProducerToReceiverTransferDataPoint.AsObject>,
    groupedrds2ctransfersList: Array<RdsToConsumerDataPoint.AsObject>,
    groupedmemorystatsList: Array<RdsMemoryDataPoint.AsObject>,
  }
}

export class BrokerDataPointContainer extends jspb.Message {
  getBrokername(): string;
  setBrokername(value: string): BrokerDataPointContainer;

  getTimestampms(): number;
  setTimestampms(value: number): BrokerDataPointContainer;

  getGroupedbrokerrequestsList(): Array<BrokerRequestDataPoint>;
  setGroupedbrokerrequestsList(value: Array<BrokerRequestDataPoint>): BrokerDataPointContainer;
  clearGroupedbrokerrequestsList(): BrokerDataPointContainer;
  addGroupedbrokerrequests(value?: BrokerRequestDataPoint, index?: number): BrokerRequestDataPoint;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): BrokerDataPointContainer.AsObject;
  static toObject(includeInstance: boolean, msg: BrokerDataPointContainer): BrokerDataPointContainer.AsObject;
  static serializeBinaryToWriter(message: BrokerDataPointContainer, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): BrokerDataPointContainer;
  static deserializeBinaryFromReader(message: BrokerDataPointContainer, reader: jspb.BinaryReader): BrokerDataPointContainer;
}

export namespace BrokerDataPointContainer {
  export type AsObject = {
    brokername: string,
    timestampms: number,
    groupedbrokerrequestsList: Array<BrokerRequestDataPoint.AsObject>,
  }
}

export class FtsToConsumerDataPointContainer extends jspb.Message {
  getFtsname(): string;
  setFtsname(value: string): FtsToConsumerDataPointContainer;

  getTimestampms(): number;
  setTimestampms(value: number): FtsToConsumerDataPointContainer;

  getGroupedfdstransfersList(): Array<FdsToConsumerDataPoint>;
  setGroupedfdstransfersList(value: Array<FdsToConsumerDataPoint>): FtsToConsumerDataPointContainer;
  clearGroupedfdstransfersList(): FtsToConsumerDataPointContainer;
  addGroupedfdstransfers(value?: FdsToConsumerDataPoint, index?: number): FdsToConsumerDataPoint;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): FtsToConsumerDataPointContainer.AsObject;
  static toObject(includeInstance: boolean, msg: FtsToConsumerDataPointContainer): FtsToConsumerDataPointContainer.AsObject;
  static serializeBinaryToWriter(message: FtsToConsumerDataPointContainer, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): FtsToConsumerDataPointContainer;
  static deserializeBinaryFromReader(message: FtsToConsumerDataPointContainer, reader: jspb.BinaryReader): FtsToConsumerDataPointContainer;
}

export namespace FtsToConsumerDataPointContainer {
  export type AsObject = {
    ftsname: string,
    timestampms: number,
    groupedfdstransfersList: Array<FdsToConsumerDataPoint.AsObject>,
  }
}

