<template>
    <div class="h-10 flex text-white border border-gray-400 ">
        <div class="timeQuickView flex flex-col h-full">
            <button class="flex-1 px-1 bg-gray-900 hover:bg-gray-500 border-b border-gray-400" @click="toggleRefreshMenu">
                <p class="text-xs">{{refreshIntervalText}}</p>
            </button>
            <button class="flex-1 px-1 bg-gray-900 hover:bg-gray-500" @click="toggleRangeMenu">
                <p class="text-xs">{{timeRangeText}}</p>
            </button>
        </div>
        <button title="Manually refresh metrics" @click="refresh()" class="max-h-full bg-gray-900 hover:bg-gray-500 p-1 border-l border-gray-400" :disabled="isFetchingData">
            <img class="refreshButtonImage" src="../assets/icons/refresh.svg" :class="isFetchingData ? 'animate-spin' : ''" />
        </button>

        <!-- TODO: Remove 'right-1' and make two divs for width -->
        <div v-if="showDisplayMenu" class="menuWithButtons absolute right-1 top-12 text-black p-2 bg-gray-300 border border-gray-600 z-10">
            <div v-if="currentDisplayIsRefreshMenu" class="flex flex-col">
                <h1 class="font-bold">Select refresh interval</h1>
                <button 
                    v-for="intervalInSec, i in refreshIntervals"
                    :key="i"
                    :class="isThisSelectedRefreshInterval(intervalInSec) ? 'selected' : ''"
                    @click="setRefreshInterval(intervalInSec)">
                        Every {{intervalInSec}} seconds
                </button>
                <div class="w-full border-b border-gray-400"></div>
                <button
                    @click="setRefreshInterval(null)"
                    :class="isThisSelectedRefreshInterval(null) ? 'selected' : ''"
                    class="highlightColor">
                        Manual mode
                </button>
            </div>
            <div v-else-if="currentDisplayIsRangeMenu" class="flex flex-col">
                <h1 class="font-bold">Select range interval</h1>
                <div class="flex flex-row">
                    <div class="flex flex-col mr-3 place-items-start">
                        <h1 class="font-bold">Custom range</h1>
                        <form class="flex flex-col ">
                            <div>
                                <label for="manul-time-from" class="w-12 inline-block">From:</label>
                                <input id="manul-time-from" class="w-24" v-model="inputTimeExpressionFrom" />
                            </div>
                            <div>
                                <label for="manul-time-to" class="w-12 inline-block">To:</label>
                                <input id="manul-time-to" class="w-24" v-model="inputTimeExpressionTo"/>
                            </div>

                            <button class="border-2 bg-gray-100 mt-2"
                                title="Evaulates the 'from' and 'to' expression and converts them to an absolute timestamp"
                                type="button"
                                @click.prevent="convertToAbsoluteTimeRange()">
                                    To absolute time
                            </button>
                            <button class="border-2 bg-gray-100 mt-1"
                                title="Applies to new time range"
                                type="submit"
                                @click.prevent="applyInputTimeRange()">
                                    Apply
                            </button>
                        </form>
                    </div>
                    <div class="flex flex-col ml-3">
                        <h1 class="font-bold">Quick select</h1>
                        <button
                            v-for="range, i in quickSelectTimesConverted"
                            :key="i"
                            @click="setTimeRangeFromTemplate(range.from, range.to)"
                            :class="isThisSelectedTimeRange(range.from, range.to) ? 'selected' : ''">
                                {{range.toHumanDescription()}}
                        </button>
                    </div>
                </div>
            </div>
        </div>
    </div>
</template>

<script lang="ts">
import { Options, Vue } from "vue-class-component";
import { connection } from '../store/connectionStore';
import { timeStore } from '../store/timeStore';
import { TimeRange } from '../lib/TimeRange';

enum TimeMenu {
    None,
    RefreshMenu,
    RangeMenu,
}

@Options({
    watch: {
        timeExpressionFrom: {
            handler() {
                this.inputTimeExpressionFrom = timeStore.state.timeExpressionFrom;
            },
            immediate: true,

        },
        timeExpressionTo: {
            handler() {
                this.inputTimeExpressionTo = timeStore.state.timeExpressionTo;
            },
            immediate: true,

        }
    }
})
export default class UpdateRateSelector extends Vue {
    private static quickSelectTimes: string[] = [
        '1m', '2m', '5m', '10m', '15m', '30m', '1h', '3h',
    ];
    private quickSelectTimesConverted = UpdateRateSelector.quickSelectTimes.map((s) => {
        return new TimeRange(`now-${s}`, 'now');
    });

    private refreshIntervals: number[] = [
        5, 10, 30, 60,
    ];

    private currentMenu: TimeMenu = TimeMenu.None;

    private inputTimeExpressionFrom!: string;
    private inputTimeExpressionTo!: string;

    private get timeExpressionFrom(): string {
        return timeStore.state.timeExpressionFrom;
    }
    private get timeExpressionTo(): string {
        return timeStore.state.timeExpressionTo;
    }

    private get showDisplayMenu(): boolean {
        return this.currentMenu != TimeMenu.None;
    }

    private get currentDisplayIsRefreshMenu(): boolean {
        return this.currentMenu == TimeMenu.RefreshMenu;
    }

    private get currentDisplayIsRangeMenu(): boolean {
        return this.currentMenu == TimeMenu.RangeMenu;
    }

    public mounted(): void {
        document.addEventListener('click', this.onWindowClick);
    }

    public beforeUnmount(): void {
        document.removeEventListener('click', this.onWindowClick);
    }

    private onWindowClick(e: MouseEvent): void {
        if (!(e as any).path.includes(this.$el)) {
            this.closeMenu();
        }
    }

    private toggleRefreshMenu(): void {
        if (this.currentMenu == TimeMenu.RefreshMenu) {
            this.currentMenu = TimeMenu.None;
        } else {
            this.currentMenu = TimeMenu.RefreshMenu;
        }
    }

    private toggleRangeMenu(): void {
        if (this.currentMenu == TimeMenu.RangeMenu) {
            this.currentMenu = TimeMenu.None;
        } else {
            this.currentMenu = TimeMenu.RangeMenu;
        }
    }

    private closeMenu(): void {
        this.currentMenu = TimeMenu.None;
    }

    private applyInputTimeRange(): void {
        timeStore.setTimeRange(this.inputTimeExpressionFrom, this.inputTimeExpressionTo);
        connection.triggerMetricsRefresh();
    }

    private convertToAbsoluteTimeRange(): void {
        this.applyInputTimeRange();
        if (timeStore.evaluateAndVerify()) {
            timeStore.setTimeRange(String(timeStore.state.lastFixedTimeRange.fromUnixSec), String(timeStore.state.lastFixedTimeRange.toUnixSec));
            connection.triggerMetricsRefresh();
        }
    }

    private setTimeRangeFromTemplate(from: string, to: string): void {
        timeStore.setTimeRange(from, to);
        connection.triggerMetricsRefresh();
    }

    private isThisSelectedTimeRange(from: string, to: string): boolean {
        return timeStore.state.timeExpressionFrom === from && timeStore.state.timeExpressionTo === to;
    }

    private setRefreshInterval(seconds: number | null): void {
        timeStore.setRefreshInterval(seconds);
        if (seconds === null) {
            connection.clearPendingMetricsTimeout();
        } else {
            connection.triggerMetricsRefresh();
        }
    }

    private isThisSelectedRefreshInterval(interval: number | null): boolean {
        return timeStore.state.refreshIntervalInSec === interval;
    }

    public get refreshIntervalInSec(): number | null {
        return timeStore.state.refreshIntervalInSec;
    }

    public get refreshIntervalText(): string {
        if (this.refreshIntervalInSec == null) {
            return `Manual refresh mode`;
        }
        return `Refresh every ${this.refreshIntervalInSec} sec`;
    }

    public get timeRangeText(): string {
        const timeRange = new TimeRange(timeStore.state.timeExpressionFrom, timeStore.state.timeExpressionTo);
        return 'Range: ' + timeRange.toHumanDescription();
    }

    get isFetchingData(): boolean {
        return connection.state.isFetchingData;
    }

    private refresh(): void {
        connection.triggerMetricsRefresh();
    }
}
</script>

<style lang="scss" scoped>
    .selected {
        @apply underline;
    }

    .highlightColor {
        @apply text-yellow-800;
        &:hover {
            @apply text-red-900;
        }
    }

    .menuWithButtons button:not(.highlightColor) {
        @apply text-blue-700;
        &:hover {
            @apply text-blue-900;
        }
    }

    .timeQuickView {
        min-width: 10em;
    }

    .refreshButtonImage {
        height: 80%;
    }
</style>