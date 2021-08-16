interface HitMissDataPoint {
    timestamp: number;
    hit: number;
    miss: number;
}

interface TransferTimePoint {
    timestamp: number;
    fromProducerToReceiverMs: number;
    receiverStaleTimeMs: number;
    fromReceiverToConsumerMs: number;
}

interface TotalTransferSpeedPoint {
    timestamp: number;
    avgBytesPerSecRecv: number;
    avgBytesPerSecSend: number;
}

interface TotalCacheMemoryUsagePoint {
    timestamp: number;
    totalUntouchedMemory: number;
    totalTouchedMemory: number;
}

export interface ToplogyStage {
    sourceName: string;
    totalFileRateRequested: number;
    totalGbRateRequested: number;

    receivers: ReceiverInfo[];
}

export interface ReceiverInfo {
    receiverId: string;
    fileRateIncoming: number;
    gbRateIncoming: number;
    usedMemory: number;
    totalMemory: number;
}

import { DataPointsQuery, DataPointsResponse, MetadataResponse, PipelineConnectionQueryMode, ToplogyQuery } from '../generated_proto/AsapoMonitoringQueryService_pb';
import { Empty } from '../generated_proto/AsapoMonitoringCommonService_pb';
import { timeStore } from './timeStore';
import { selectionFilterStore } from './selectionFilterStore';

interface ConnectionStoreState {
    isFetchingMetaData: boolean;
    isFetchingData: boolean;
    isFetchingToplogyData: boolean;

    // metadata
    clusterName: string;
    availableBeamtimes: string[];

    // metrics
    hitMissDataPoints: HitMissDataPoint[];
    transferTimePoints: TransferTimePoint[];
    transferSpeed: TotalTransferSpeedPoint[];
    cacheMemoryUsage: TotalCacheMemoryUsagePoint[];
    toplogyStages: ToplogyStage[];

    // TODO: Map them here
    data: DataPointsResponse | null;

    // topology
    // TODO    
}


import { reactive } from 'vue';
import { AsapoMonitoringQueryServiceClient } from '../generated_proto/AsapoMonitoringQueryService_grpc_web_pb';
import { errorStore } from './errorStore';
import { toplogyStore } from './toplogyStore';

console.log('AsapoMonitoringQueryServicePromiseClient');
const client = new AsapoMonitoringQueryServiceClient('/api', null);

class ConnectionStore {
    private pendingDataRefreshTimeoutHandle?: number;

    private internalState: ConnectionStoreState = reactive({
        isFetchingMetaData: false,
        isFetchingData: false,
        isFetchingToplogyData: false,

        clusterName: '<NOT SET>',
        availableBeamtimes: [],

        data: null,

        hitMissDataPoints: [],
        transferTimePoints: [],
        transferSpeed: [],
        cacheMemoryUsage: [],

        toplogyStages: [
            {
                sourceName: 'TestSource',
                totalFileRateRequested: 998,
                totalGbRateRequested: 0.87,
                receivers: [
                    {
                        receiverId: 'fakeReceiver-1',
                        usedMemory: 1024*1024*940,
                        totalMemory: 1024*1024*1000,
                        fileRateIncoming: 605,
                        gbRateIncoming: 0.54,
                    },
                    {
                        receiverId: 'fakeReceiver-2',
                        usedMemory: 1024*1024*500,
                        totalMemory: 1024*1024*1000,
                        fileRateIncoming: 395,
                        gbRateIncoming: 0.35,
                    },
                ]
            }
        ]
    });

    public get state(): Readonly<ConnectionStoreState> {
        return this.internalState;
    }

    public setMetadata(initialData: MetadataResponse): void {
        this.internalState.clusterName = initialData.getClustername();
        this.internalState.availableBeamtimes = initialData.getBeamtimeList();
    }

    public setData(data: DataPointsResponse): void {
        // TODO: Map data to plot data here
        this.internalState.data = data;
    }

    public setIsFetchingData(mode: boolean): void {
        this.internalState.isFetchingData = mode;
    }

    public setIsFetchingMetaData(mode: boolean): void {
        this.internalState.isFetchingMetaData = mode;
    }

    public setIsFetchingToplogyData(mode: boolean): void {
        this.internalState.isFetchingToplogyData = mode;
    }

    public triggerToplogyRefresh(): void {
        if (this.state.isFetchingToplogyData) {
            console.warn('Unable to fetch toplogy: Already fetching toplogy data.');
            return; // already fetching data
        }

        if (!selectionFilterStore.isValidFilterOrSetError()) {
            return;
        }
       
        this.setIsFetchingToplogyData(true);

        const query = new ToplogyQuery();

        const range = timeStore.state.lastFixedTimeRange;
        query.setFromtimestamp(range.fromUnixSec);
        query.setTotimestamp(range.toUnixSec);
        query.setBeamtimefilter(selectionFilterStore.state.beamtime!);
    
        errorStore.clearToplogyConnectionError();
        client.getTopology(query, undefined, (err, toplogyData) => {
            if (err) {
                errorStore.setToplogyConnectionErrorText(err.message);
                console.error('getTopology error: ', err);
                this.setIsFetchingToplogyData(false);
                return;
            }
            
            try {
                toplogyStore.parseServerResponse(toplogyData);
            }
            catch(e) {
                errorStore.setToplogyConnectionErrorText('internal: ' + e.message);
                console.error('getTopology internal error: ', e);
            }
            finally {
                this.setIsFetchingToplogyData(false);
            }

        });
    }

    public triggerMetaDataRefresh(): void {
        if (this.state.isFetchingMetaData) {
            console.warn('Unable to fetch metadata: Already fetching metadata.');
            return; // already fetching data
        }

        this.setIsFetchingMetaData(true);

        errorStore.clearMetadataConnectionError();
        client.getMetadata(new Empty(), undefined, (err, metaData) => {
            if (err) {
                errorStore.setMetadataConnectionErrorText(err.message);
                console.error('getMetadata error: ', err);
                this.setIsFetchingMetaData(false);
                return;
            }

            this.setMetadata(metaData);
            this.setIsFetchingMetaData(false);
        });
    }

    public clearPendingMetricsTimeout(): void {
        if (this.pendingDataRefreshTimeoutHandle !== undefined) { // remove pending delay
            clearTimeout(this.pendingDataRefreshTimeoutHandle);
            this.pendingDataRefreshTimeoutHandle = undefined;
        }
    }

    public triggerMetricsRefresh(reevaluateTime = true): void {
        if (this.state.isFetchingData) {
            console.warn('Unable to fetch data: Already fetching data.');
            return; // already fetching data
        }
        this.clearPendingMetricsTimeout();
        if (!selectionFilterStore.isValidFilterOrSetError()) {
            return;
        }

        if (reevaluateTime) {
            if (!timeStore.evaluateAndVerify()) {
                return;
            }
        }

        if (toplogyStore.couldBeOutdated()) {
            this.triggerToplogyRefresh();
        }

        this.setIsFetchingData(true);
        
        const query = new DataPointsQuery();
    
        query.setFromtimestamp(timeStore.state.lastFixedTimeRange.fromUnixSec);
        query.setTotimestamp(timeStore.state.lastFixedTimeRange.toUnixSec);
    
        if (selectionFilterStore.state.beamtime) {
            query.setBeamtimefilter(selectionFilterStore.state.beamtime);
        }
        if (selectionFilterStore.state.source) {
            query.setSourcefilter(selectionFilterStore.state.source);
        }
        if (selectionFilterStore.state.stream) {
            query.setStreamfilter(selectionFilterStore.state.stream);
        }
        if (selectionFilterStore.state.fromPipelineStepId) {
            query.setFrompipelinestepfilter(selectionFilterStore.state.fromPipelineStepId);
        }
        if (selectionFilterStore.state.toPipelineStepId) {
            query.setTopipelinestepfilter(selectionFilterStore.state.toPipelineStepId);
        }
        if (selectionFilterStore.state.fromPipelineStepId || selectionFilterStore.state.toPipelineStepId) {
            query.setPipelinequerymode(selectionFilterStore.state.pipelineFilterRelation === 'and' ? PipelineConnectionQueryMode.EXACTPATH : PipelineConnectionQueryMode.JUSTRELATEDTOSTEP);
        }
        if (selectionFilterStore.state.receiverId) {
            query.setReceiverfilter(selectionFilterStore.state.receiverId);
        }

        errorStore.clearMetricsConnectionError();
        client.getDataPoints(query, undefined, (err, data) => {
            if (err) {
                errorStore.setMetricsConnectionErrorText(err.message);
                console.error('getDataPoints error: ', err);
                this.setIsFetchingData(false);
                return;
            }

            this.setData(data);
            if (timeStore.state.refreshIntervalInSec) {
                this.pendingDataRefreshTimeoutHandle = setTimeout(() => this.triggerMetricsRefresh(), timeStore.state.refreshIntervalInSec * 1000);
            }
            this.setIsFetchingData(false);
        });
    }
}

// Singelton instance
export const connection = new ConnectionStore();

connection.triggerMetaDataRefresh();
if (selectionFilterStore.state.beamtime) {
    connection.triggerMetricsRefresh();
    connection.triggerToplogyRefresh();
}

