import { reactive } from "vue";
import { DataPointsResponse } from "../generated_proto/AsapoMonitoringQueryService_pb";

export type MetricDataPoint = [number/*Timestamp*/, number/*Value*/]
export type MetricDataSeries = MetricDataPoint[/*timestamp index*/]
export type MetricDataComplete = MetricDataSeries[/*series index*/]

export interface MetricDataStoreState {
    transferSpeedChartData: MetricDataComplete;
    detailedTransferSpeedChartData: MetricDataComplete;
    fileRateChartData: MetricDataComplete;
    receiverTimePerTaskChartData: MetricDataComplete;
    receiverRamUsageChartData: MetricDataComplete;
}

class MetricDataStore {
    private internalState: MetricDataStoreState = reactive({
        transferSpeedChartData: [],
        detailedTransferSpeedChartData: [],
        fileRateChartData: [],
        receiverTimePerTaskChartData: [],
        receiverRamUsageChartData: [],
    });

    public get state(): Readonly<MetricDataStoreState> {
        return this.internalState;
    }

    public parseServerResponse(data: DataPointsResponse): void {
        const interval = data.getTimeintervalinsec()

        // transferSpeed & detailedTransferSpeed
        {
            let currentTime = data.getStarttimestampinsec();
            const transferSpeedRecv = [] as MetricDataSeries;
            const transferSpeedSend = [] as MetricDataSeries;
            const transferSpeedRdsSend = [] as MetricDataSeries;
            const transferSpeedFtsSend = [] as MetricDataSeries;
            for (const point of data.getTransferratesList()) {
                transferSpeedRecv.push([currentTime * 1000, point.getTotalbytespersecrecv()]);
                transferSpeedSend.push([currentTime * 1000, point.getTotalbytespersecsend()]);
                transferSpeedRdsSend.push([currentTime * 1000, point.getTotalbytespersecrdssend()]);
                transferSpeedFtsSend.push([currentTime * 1000, point.getTotalbytespersecftssend()]);

                currentTime += interval;
            }

            this.internalState.transferSpeedChartData = [transferSpeedRecv, transferSpeedSend];
            this.internalState.detailedTransferSpeedChartData = [transferSpeedRecv, transferSpeedRdsSend, transferSpeedFtsSend];
        }

        // fileRate
        {
            let currentTime = data.getStarttimestampinsec();
            const unknown = [] as MetricDataSeries;
            const miss = [] as MetricDataSeries;
            const fromCache = [] as MetricDataSeries;
            const fromDisk = [] as MetricDataSeries;
            for (const point of data.getFileratesList()) {
                const knownSum = point.getFromcache() + point.getFromdisk();
                unknown.push([currentTime * 1000, Math.max(0, point.getTotalrequests() - knownSum)]);
                fromCache.push([currentTime * 1000, point.getFromcache()]);
                fromDisk.push([currentTime * 1000, point.getFromdisk()]);

                miss.push([currentTime * 1000, point.getCachemisses()]);
                /*
                if (miss.length > 2 && !miss[miss.length - 1][1] && !miss[miss.length - 2][1] && point.getCachemisses() === 0) {
                    miss[miss.length - 1][1] = undefined as any;
                    miss.push([currentTime * 1000, 0]);
                } else {
                    miss.push([currentTime * 1000, point.getCachemisses()]);
                }*/

                currentTime += interval;
            }

            this.internalState.fileRateChartData = [unknown, miss, fromCache, fromDisk];
        }

        // receiverTimePerTask
        {
            let currentTime = data.getStarttimestampinsec();
            const receiveIo = [] as [number, number][];
            const writeIo = [] as [number, number][];
            const databaseIo = [] as [number, number][];
            for (const point of data.getTasktimesList()) {

                receiveIo.push([currentTime * 1000, point.getReceiveiotimeus()]);
                writeIo.push([currentTime * 1000, point.getWritetodisktimeus()]);
                databaseIo.push([currentTime * 1000, point.getWritetodatabasetimeus()]);

                currentTime += interval;
            }

            this.internalState.receiverTimePerTaskChartData = [receiveIo, writeIo, databaseIo];
        }

        // receiverRamUsage
        {
            let currentTime = data.getStarttimestampinsec();
            const memoryUsed = [] as [number, number][];
            for (const point of data.getMemoryusagesList()) {

                memoryUsed.push([currentTime * 1000, point.getTotalusedmemory()]);

                currentTime += interval;
            }

            this.internalState.receiverRamUsageChartData = [memoryUsed];
        }
    }
}

export const metricData = new MetricDataStore();
