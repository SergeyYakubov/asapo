<template>
    <div id="dashboard" class="w-full h-full flex flex-col">
        <NavBar />
        <main class="flex-1 flex m-2">
            <div class="flex-1 flex flex-col"><!-- left side -->
            <!--flex-grow: 1; -->
                <TopologyGraph style="height: 97%" />
                <!--<LiveMessageLog style=" height: 33.333%;" />-->
            </div>
            <div class="flex-1"><!-- right side -->
                <div class="graph-group">
                    <GenericChart class="inline-flex" ref="transferSpeedChart" :chartInfo="{
                        title: 'Total transfer speed',
                        yUnit: 'byteRate',
                        stacked: false,
                        series: [
                            { name: 'Produced', description: 'Total input bytes per seconds for all selected receivers', color: '#edc240' },
                            { name: 'Consumed (requested)', description: 'Total requested outgoing bytes (consumer requested broker)', color: '#afd8f8' },
                        ]}" />
                    <GenericChart class="inline-flex" ref="fileRateChart" :chartInfo="{
                        title: 'Total file rate',
                        yUnit: 'plainNumber',
                        stacked: true,
                        series: [
                            { name: 'Unknown', description: 'Requested but unknown how they were resolved (possible local FS)', color: '#6d6d6d' },
                            { name: 'Cache Miss', description: 'The Receiver Data Service got an request but was not able to find this file in the cache', color: '#ed6240' },
                            { name: 'From cache', description: 'The Receiver Data Service got an request and was able to serve the file', color: '#4dc240' },
                            { name: 'From disk (FTS)', description: 'The File Transfer Service got an request and was able to serve the file', color: '#2196f3' },
                        ]}" />
                    <GenericListChart class="inline-flex" label="Producers" :elements="involvedProducers" />
                    <GenericListChart class="inline-flex" label="Consumers" :elements="involvedConsumers" />
                </div>
                
                <h2 class="ml-1 mt-2">Advanced stats</h2>
                <div class="graph-group">
                    <GenericListChart class="inline-flex" label="Involved Receviers" :elements="involvedReceiverList" :onElementClicked="onReceiverSelected" :isElementSelected="isReceiverSelected" />
                    <GenericChart class="inline-flex" ref="detailedTransferSpeedChart" :chartInfo="{
                        title: 'Total transfer speed by service',
                        yUnit: 'byteRate',
                        stacked: false,
                        series: [
                            { name: 'Input', description: 'Total input bytes per seconds for all selected receivers', color: '#edc240' },
                            { name: 'RDS Output', description: 'Total (RDS)output bytes per seconds for all selected receivers', color: '#4dc240' },
                            { name: 'FTS Output', description: 'Total (FTS)output bytes per seconds', color: '#2196f3' },
                        ]}" />
                    <GenericChart class="inline-flex w-full" ref="receiverTimePerTaskChart" :chartInfo="{
                        title: 'Receiver: Average time per task',
                        yUnit: 'timeUs',
                        stacked: true,
                        series: [
                            { name: 'Receive IO', description: 'Time it took to transfer the file from the consumer to the receiver', color: '#edc240' },
                            { name: 'Write IO', description: 'Time it took to write the file to the disk', color: '#c7619a' },
                            { name: 'Database', description: 'Time it took to write the file-metadata to the database', color: '#6d7cd0' },
                            { name: 'RDS: Send IO', description: 'Time it took to send the file to a consumer (decoupled from other values)', color: '#4dc240' },
                        ]}" />
                    <GenericChart class="inline-flex" ref="receiverRamUsageChart" :chartInfo="{
                        title: 'Receiver: Cache memory usage',
                        yUnit: 'byte',
                        stacked: true,
                        series: [
                            { name: 'Used', description: 'Memory used by current selection', color: '#673ab7' },              
                        ]}" />
                </div>
            </div>
        </main>
    </div>
</template>

<script lang="ts">
import { Options, Vue } from 'vue-class-component';
import NavBar from '../components/NavBar.vue';
import TopologyGraph from '../components/TopologyGraph.vue';
import GenericChart from '../components/GenericChart.vue';
import GenericListChart from '../components/GenericListChart.vue';
import { connection } from '../store/connectionStore';
import { DataPointsResponse } from '../generated_proto/AsapoMonitoringQueryService_pb';
import { selectionFilterStore } from '../store/selectionFilterStore';
import { toplogyStore } from '../store/toplogyStore';
import { metricData, MetricDataComplete } from '../store/metricDataStore';


@Options({
    components: {
        NavBar,

        TopologyGraph,
        GenericChart,
        GenericListChart,
    },
    watch: {
        storeData() {
            if (this.storeData != null) {
                this.onUpdateData(this.storeData);
            }
        },
        transferSpeedChartData() {
            this.onTransferSpeedChartDataChanged();
        },
        detailedTransferSpeedChartData() {
            this.onDetailedTransferSpeedChartDataChanged();
        },
        fileRateChartData() {
            this.onFileRateChartDataChanged();
        },
        receiverTimePerTaskChartData() {
            this.onReceiverTimePerTaskChartDataChanged();
        },
        receiverRamUsageChartData() {
            this.onReceiverRamUsageChartDataChanged();
        },
    }
})
export default class Dashboard extends Vue {
    public get transferSpeedChartData(): MetricDataComplete {
        return metricData.state.transferSpeedChartData;
    }
    public get detailedTransferSpeedChartData(): MetricDataComplete {
        return metricData.state.detailedTransferSpeedChartData;
    }
    public get fileRateChartData(): MetricDataComplete {
        return metricData.state.fileRateChartData;
    }
    public get receiverTimePerTaskChartData(): MetricDataComplete {
        return metricData.state.receiverTimePerTaskChartData;
    }
    public get receiverRamUsageChartData(): MetricDataComplete {
        return metricData.state.receiverRamUsageChartData;
    }

    private get involvedProducers(): string[] {
        let nodes = toplogyStore.state.pipeline.nodes;
        
        if (selectionFilterStore.state.fromPipelineStepId && selectionFilterStore.state.toPipelineStepId) {
            nodes = nodes.filter(node => node.id === selectionFilterStore.state.fromPipelineStepId || node.id === selectionFilterStore.state.toPipelineStepId);
        }

        return nodes.flatMap(node => node.producers);
    }

    private get involvedConsumers(): string[] {
        let nodes = toplogyStore.state.pipeline.nodes;
        
        if (selectionFilterStore.state.fromPipelineStepId && selectionFilterStore.state.toPipelineStepId) {
            nodes = nodes.filter(node => node.id === selectionFilterStore.state.fromPipelineStepId || node.id === selectionFilterStore.state.toPipelineStepId);
        }

        return nodes.flatMap(node => node.consumers);
    }

    private get involvedReceiverList(): string[] {
        let edges = toplogyStore.state.pipeline.edges;

        if (selectionFilterStore.state.source) {
            edges = edges.filter(edge => edge.sourceName === selectionFilterStore.state.source);
        }
        if (selectionFilterStore.state.receiverId) {
            edges = edges.filter(edge => edge.receivers.includes(selectionFilterStore.state.receiverId!));
        }
        if (selectionFilterStore.state.fromPipelineStepId && selectionFilterStore.state.toPipelineStepId) {
            edges = edges.filter(edge => edge.fromId === selectionFilterStore.state.fromPipelineStepId && edge.toId === selectionFilterStore.state.toPipelineStepId);
        }

        const receiverIds = edges.flatMap(edge => edge.receivers);
    
        return [...new Set(receiverIds)]; // Make unique
    }

    private onReceiverSelected(receiverId: string): void {
        selectionFilterStore.setFilterReceiverId(receiverId);
    }

    private isReceiverSelected(receiverId: string): boolean {
        return selectionFilterStore.state.receiverId === receiverId;
    }

    private onTransferSpeedChartDataChanged(): void {
            const chartRef = this.$refs['transferSpeedChart'] as GenericChart;
            chartRef.setDataPoints(this.transferSpeedChartData);
    }
    private onDetailedTransferSpeedChartDataChanged(): void {
            const chartRef = this.$refs['detailedTransferSpeedChart'] as GenericChart;
            chartRef.setDataPoints(this.detailedTransferSpeedChartData);
    }
    private onFileRateChartDataChanged(): void {
            const chartRef = this.$refs['fileRateChart'] as GenericChart;
            chartRef.setDataPoints(this.fileRateChartData);
    }
    private onReceiverTimePerTaskChartDataChanged(): void {
            const chartRef = this.$refs['receiverTimePerTaskChart'] as GenericChart;
            chartRef.setDataPoints(this.receiverTimePerTaskChartData);
    }
    private onReceiverRamUsageChartDataChanged(): void {
            const chartRef = this.$refs['receiverRamUsageChart'] as GenericChart;
            chartRef.setDataPoints(this.receiverRamUsageChartData);
    }
}
</script>

<!-- Add "scoped" attribute to limit CSS to this component only -->
<style scoped lang="scss">
h3 {
    margin: 40px 0 0;
    span {
        color: red;
    }
}
ul {
    list-style-type: none;
    padding: 0;
}
li {
    display: inline-block;
    margin: 0 10px;
}
a {
    color: #42b983;
}

@media (min-width: 1200px) { 
    .graph-group > * {
        width: 50%;
    }
}
@media (max-width: 1200px) { 
    .graph-group > * {
        width: 100%;
    }
}
</style>
