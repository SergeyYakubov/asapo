<template>
    <div class='border p-1 relative' :class="hasFilterError ? 'hasError' : ''">
        <div class="flex items-center" @click="togglePopup">
            <div class="flex flex-row">
                <p class="place-self-center mr-2">Filter:</p>
                <div class="flex font-mono text-xs">
                    <div class="mr-2">
                        <table>
                            <tr>
                                <td>Source:</td>
                                <td>{{currentSourceFilterText}}</td>
                            </tr>
                        </table>
                    </div>
                    <div>
                        <table>
                            <tr>
                                <td>Receiver:</td>
                                <td>{{currentReceiverFilterText}}</td>
                            </tr>
                            <tr>
                                <td>Pipeline:</td>
                                <td>{{currentPipelineFilterText}}</td>
                            </tr>
                        </table>
                    </div>
                </div>
            </div>
            <button class="text-white p-1 ml-2" :class="hasClearableFilter ? 'bg-red-700 hover:bg-red-800' : 'bg-gray-600'" @click.stop="clearFilter()" :disabled="!hasClearableFilter">Clear</button>
        </div>
        <div v-if="showPopup" class="absolute top-12 text-black p-2 bg-gray-300 border border-gray-600 z-10 w-max right-0">
            <div class="flex justify-between">
                <h1 class="font-bold text-lg">Select filter:</h1>
                <button :disabled="isFetchingData" @click="reloadMetaDataOrToplogy()">
                    <img src="../assets/icons/single_refresh.svg" class="w-8 h-8" :class="isFetchingData ? 'animate-spin' : ''" />
                </button>
            </div>
            <div class="flex flex-col">
                <div>
                    <label for="beamtime" class="inline-block w-20">Beamtime:</label>
                    <select name="Beamtime" id="beamtime" class="w-32" v-model="selectedBeamtime">
                        <option v-for="beamtime in availableBeamtimes" :key="beamtime">{{beamtime}}</option>
                    </select>
                    <button @click="clearBeamtimeFilter()">X</button>
                </div>
            </div>

            <div class="mt-3 flex flex-col">
                <p class="font-bold">For metrics</p>
                <div>
                    <label for="source" class="inline-block w-20">Source:</label>
                    <select name="Source" id="source" class="w-32" v-model="selectedSource">
                        <option v-for="source in availableSources" :key="source">{{source}}</option>
                    </select>
                    <button @click="clearSourceFilter()">X</button>
                </div>
            </div>
            <div class="mt-3 flex flex-col">
                <p class="font-bold">Pipeline connection</p>
                <table>
                    <tr>
                        <td>From Step:</td>
                        <td><input type="text" id="source" disabled class="w-28" :value="currentPipelineStepFrom"></td>
                    </tr>
                    <!--
                    <tr>
                        <td>Source:</td>
                        <td><input type="text" id="source" disabled class="w-28"></td>
                    </tr>
                    -->
                    <tr>
                        <td>To Step:</td>
                        <td><input type="text" id="source" disabled class="w-28" :value="currentPipelineStepTo"></td>
                    </tr>
                </table>
            </div>
        </div>
    </div>
</template>

<script lang="ts">
import { Options, Vue } from "vue-class-component";
import { connection } from "../store/connectionStore";
import { errorStore } from "../store/errorStore";
import { selectionFilterStore } from "../store/selectionFilterStore";
import { toplogyStore } from "../store/toplogyStore";

@Options({
    watch: {
        selectedBeamtimeFromStore: {
            handler(newValue: string | null): void {
                this.selectedBeamtime = newValue;
            },
            immediate: true,
        },
        selectedBeamtime(newValue: string | null): void {
            selectionFilterStore.setFilterBeamtime(newValue);
        },
        currentBeamtimeFilterText(): void {
            this.selectedBeamtime = selectionFilterStore.state.beamtime;
        },
        selectedSourceFromStore(newValue: string | null): void {
            this.selectedSource = newValue;
        },
        selectedSource(newValue: string | null): void {
            selectionFilterStore.setFilterSource(newValue);
        },
        currentSourceFilterText(): void {
            this.selectedSource = selectionFilterStore.state.source;
        },
    }
})
export default class FilterSelector extends Vue {
    private selectedBeamtime: string | null = null;
    private selectedSource: string | null = null;
    private showPopup: boolean = false;

    private get selectedBeamtimeFromStore(): string | null {
        return selectionFilterStore.state.beamtime;
    }
    private get selectedSourceFromStore(): string | null {
        return selectionFilterStore.state.source;
    }

    private get hasClearableFilter(): boolean {
        return selectionFilterStore.hasClearableFilter;
    }
    
    public get availableBeamtimes(): string[] {
        return connection.state.availableBeamtimes;
    }

    public get availableSources(): string[] {
        return toplogyStore.getAvailableSources();
    }

    public mounted(): void {
        document.addEventListener('click', this.onWindowClick);
    }

    public beforeUnmount(): void {
        document.removeEventListener('click', this.onWindowClick);
    }

    private reloadMetaDataOrToplogy() {
        connection.triggerMetaDataRefresh();
        if (selectionFilterStore.state.beamtime) {
            connection.triggerToplogyRefresh();
        }
    }

    private get isFetchingData(): boolean {
        return connection.state.isFetchingMetaData || connection.state.isFetchingToplogyData;
    }

    private togglePopup(): void {
        this.showPopup = !this.showPopup;
    }

    private onWindowClick(e: MouseEvent): void {
        if (!(e as any).path.includes(this.$el)) {
            this.showPopup = false;
        }
    }

    private clearFilter(): void {
        selectionFilterStore.clearFilter();
    }

    private get hasFilterError(): boolean {
        return !!errorStore.state.filterErrorText;
    }

    private get currentBeamtimeFilterText(): string {
        if (selectionFilterStore.state.beamtime) {
            return selectionFilterStore.state.beamtime;
        }
        return '*';
    }

    private clearBeamtimeFilter(): void {
        selectionFilterStore.setFilterBeamtime(null);
    }

    private clearSourceFilter(): void {
        selectionFilterStore.clearSourceFilter();
    }

    private get currentSourceFilterText(): string {
        if (selectionFilterStore.state.source) {
            return selectionFilterStore.state.source;
        }
        return '*';
    }

    private get currentReceiverFilterText(): string {
        if (selectionFilterStore.state.receiverId) {
            return selectionFilterStore.state.receiverId;
        }
        return '*';
    }

    private get currentPipelineStepFrom(): string | null {
        if (selectionFilterStore.state.fromPipelineStepId) {
            return selectionFilterStore.state.fromPipelineStepId;
        }
        return '';
    }

    private get currentPipelineStepTo(): string | null {
        if (selectionFilterStore.state.toPipelineStepId) {
            return selectionFilterStore.state.toPipelineStepId;
        }
        return '';
    }

    private get currentPipelineFilterText(): string {
        if (this.currentPipelineStepFrom || this.currentPipelineStepTo) {
            if (selectionFilterStore.state.pipelineFilterRelation === 'and') {
                return `${this.currentPipelineStepFrom} ➜ ${this.currentPipelineStepTo}`;
            } else {
                return `${this.currentPipelineStepFrom}@${this.currentPipelineStepTo}`;
            }
        }
        return '';
    }
}
</script>

<style lang="scss" scoped>
    .hasError {
        @apply border-red-500;
    }
</style>
