<template>
    <div>
        <div class="w-full h-full bg-gray-100 border border-gray-600 m-1 p-1">
            <div class="flex place-content-between">
                <p class="font-bold">{{chartInfo.title}}</p>
            </div>
            <div ref="diagram" style="width: 100%; height: 170px"></div>
            <div>
                <div class="mx-2 inline-block" v-for="(series) in chartInfo.series" :key="series.color" :title="series.description">
                    <div class="inline-block w-4 h-3" :style="{backgroundColor: series.color}"></div>
                    <span class="text-xs ml-1">{{series.name}}</span>
                </div>
            </div>
        </div>
    </div>
</template>

<script lang="ts">
import { Options, Vue } from "vue-class-component";
import $, { plot } from 'jquery';
import Moment from 'moment';
import { tooltipStore, TooltipTableInfo } from "../store/tooltipStore";
import { chartCrosshairStore } from "../store/chartCrosshairStore";

interface SeriesInfo {
    name: string;
    description: string;
    color: string;
}

interface ChartInfo {
    title: string;
    yUnit: 'byteRate' | 'byte' | 'timeMs' | 'timeUs' | 'plainNumber';
    stacked: boolean;
    series: SeriesInfo[];
}

type SeriesDataPoint = [number/* timestamp */, number /* datapoint */];
type SeriesData = SeriesDataPoint[];

@Options({
    props: {
        chartInfo: Object // ChartInfo
    },
    watch: {
        chartInfo(): void {
            if (this.plot) {
                const data = this.plot.getData();
                this.destroyPlot();
                this.setupPlot();
                this.setDataPoints(data);
            }
        },
        crosshairPosX(newX): void {
            if (this.plot) {
                if (newX) {
                    this.plot.setCrosshair({x: newX});
                }
                else {
                    this.plot.clearCrosshair()
                }
            }
        }
    }
})
export default class GenericChart extends Vue {
    $plot!: JQuery<HTMLElement>;
    plot!: jquery.flot.plot;

    public chartInfo!: ChartInfo;

    public mounted() {
        this.setupPlot();
    }

    public beforeUnmount(): void {
        this.destroyPlot();
    }

    public setDataPoints(data: SeriesData[]): void {
        if (this.plot) {
            this.plot.setData(data);

            if (data[0] && data[0][data[0].length - 1] && data[0][data[0].length - 1][0]) {
                const lastTimestamp = data[0][data[0].length - 1][0];
                const timeDiffMs = Date.now() - new Date(lastTimestamp).getTime();
                const toBeDisplayedDiff = 8000 - timeDiffMs // ~ 8 sec
                if (toBeDisplayedDiff > 0) {  
                    this.plot.getOptions()!.grid!.markings = [
                        { xaxis: { from: lastTimestamp - toBeDisplayedDiff }, color: "#a9a9a9" },
                    ];
                }
            }

            this.plot.setupGrid();
            this.plot.draw();
        }
    }

    private get crosshairPosX(): number | null {
        return chartCrosshairStore.state.xValue;
    }

    private setupPlot(): void {
        const options = {
            // xaxis: is handeld by customXAxisTickGenerator and customXAxisTickFormatter
            xaxis: {
                timezone: null,         // "browser" for local to the client or timezone for timezone-js
                timeformat: null,       // format string to use
                twelveHourClock: false, // 12 or 24 time in time mode
                monthNames: null,       // list of names of months
                timeBase: 'milliseconds'     // are the values in given in mircoseconds, milliseconds or seconds
            } as any,
            yaxis: {
                tickDecimals:  0,
                labelWidth: 65,
                min: 0.001, // Prevent 0 display
            },
            series: {
                stack: this.chartInfo.stacked,
                lines: {
                    show: true,
                    fill: true,
                },
            },
            crosshair: {
                mode: 'x',
            },
            grid: {
                mouseActiveRadius: 0,
                hoverable: true,
                backgroundColor: '#f5f5f5',
            },
            colors: this.chartInfo.series.map(s => s.color),
        } as jquery.flot.plotOptions;

        if (this.chartInfo.yUnit == 'byteRate') {
            options.yaxis!.mode = 'byteRate';
            options.yaxis!.tickDecimals = 1;
        } else if (this.chartInfo.yUnit == 'byte') {
            options.yaxis!.mode = 'byte';
            options.yaxis!.tickDecimals = 1;
        } else if (this.chartInfo.yUnit == 'timeMs') {
            options.yaxis!.tickFormatter = (t: number): string => `${t} ms`;
        } else if (this.chartInfo.yUnit == 'timeUs') {
            options.yaxis!.tickFormatter = (t: number): string => `${t} μs`;
        }

        this.$plot = $(this.$refs.diagram as any);
        this.plot = $.plot(this.$plot, [], options);

        (this.plot.getAxes().xaxis as any).tickGenerator = GenericChart.customXAxisTickGenerator;
        (this.plot.getAxes().xaxis as any).tickFormatter = GenericChart.customXAxisTickFormatter;

        window.addEventListener('resize', this.redraw);
        this.$plot.on('plothover', this.onPlotHover);
        this.$plot.on('mouseout', this.onMouseLeave);

        this.redraw();
    }

    private destroyPlot(): void {
        window.removeEventListener('resize', this.redraw);
        if (this.$plot) {
            this.$plot.off('plothover', this.onPlotHover);
            this.$plot.off('mouseout', this.onMouseLeave);
        }
    }

    private redraw(): void {
        this.plot.resize();
        this.plot.setupGrid();
        this.plot.draw();
    }

    private onPlotHover(jQueryEvent: any, mousePos: {x: number, y: number, pageX: number, pageY: number}): void {
        const rawPlotData = this.plot.getData();
        if (rawPlotData[0]) {
            const timelineX = Math.trunc(mousePos.x);
            const timelineData = rawPlotData[0].data;

            if (timelineData.length < 1) {
                return;
            }

            let i;
            for (i = 0; i < timelineData.length - 1; i++) {
                const distanceToThis = Math.abs(timelineData[i][0] - timelineX);
                const distanceToNext = Math.abs(timelineData[i + 1][0] - timelineX);
                if (distanceToNext > distanceToThis) {
                    break;
                }
            }

            const timestamp = timelineData[i][0];

            chartCrosshairStore.showAndUpdate(timestamp);
            (this.plot as any).lockCrosshair({x: timestamp});

            const axes = this.plot.getAxes();
            const xAxis = axes.xaxis;
            const yAxis = axes.yaxis;

            const outLines = [] as TooltipTableInfo[];
            for (let seriesIndex = 0; seriesIndex < rawPlotData.length; seriesIndex++) {
                const series = rawPlotData[seriesIndex];
                const value = series.data[i][1];

                let yTextValue;
                if (this.chartInfo.yUnit == 'byteRate' || this.chartInfo.yUnit == 'byte') {
                    yTextValue = yAxis.tickFormatter!(value, {tickDecimals: -2} as any);
                } else {
                    yTextValue = yAxis.tickFormatter!(value, yAxis);
                }

                outLines.push({
                    name: this.chartInfo.series[seriesIndex].name,
                    value: yTextValue
                });
            }

            const xTextValue = xAxis.tickFormatter!(timestamp, xAxis);
            tooltipStore.showAndUpdate(mousePos.pageX, mousePos.pageY, xTextValue, outLines);
        }
    }

    private onMouseLeave(): void {
        chartCrosshairStore.hide();
        tooltipStore.hide();
    }

    private static customXAxisTickGenerator(axis: any): any[] {
        const tickAmount = 5;
        const totalTimeRangeMs = axis.datamax - axis.datamin;
        const deltaPerTick = totalTimeRangeMs / (tickAmount - 1);
        
        const tickPoints = Array(tickAmount);
        for (let i = 0; i < tickAmount; i++) {
            tickPoints[i] = axis.datamin + (deltaPerTick * i);
        }

        return tickPoints.map((x) => [x, axis.tickFormatter(x, axis, totalTimeRangeMs < 120 * 1000)]);
    }

    private static customXAxisTickFormatter(timestamp: number, axis: any, isDetails = true) {
        if (isDetails) {
            return Moment((timestamp || 0)).format('HH:mm:ss');
        } else {
            return Moment((timestamp || 0)).format('HH:mm');
        }
    }
}
</script>

<style>

</style>
