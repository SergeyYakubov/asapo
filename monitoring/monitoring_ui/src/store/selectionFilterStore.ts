import { reactive } from "vue";
import { connection } from "./connectionStore";
import { errorStore } from "./errorStore";

const localStorageLastBeamtimeKey = 'selected-beamtime';

export interface SelectionFilterStoreState {
    beamtime: string | null;

    source: string | null;
    stream: string | null;

    fromPipelineStepId: string | null;
    toPipelineStepId: string | null;
    pipelineFilterRelation: 'and' | 'or';
    receiverId: string | null;
}

class SelectionFilterStore {
    private internalState: SelectionFilterStoreState = reactive({
        beamtime: null,
        source: null,
        stream: null,
        fromPipelineStepId: null,
        toPipelineStepId: null,
        pipelineFilterRelation: 'and',
        receiverId: null,
    });

    public get state(): Readonly<SelectionFilterStoreState> {
        return this.internalState;
    }
    
    public get hasClearableFilter(): boolean {
        return !!(this.state.receiverId || this.state.source || this.state.stream || this.state.fromPipelineStepId || this.state.toPipelineStepId);
    }

    public setFilterBeamtime(beamtime: string | null, isInitial = false): void {
        this.internalState.beamtime = beamtime;
        this.internalState.source = null;
        this.internalState.receiverId = null;

        if (beamtime != null) {
            localStorage.setItem(localStorageLastBeamtimeKey, beamtime);
        } else {
            localStorage.removeItem(localStorageLastBeamtimeKey);
        }

        if (!isInitial) { // otherwise we would get a dependency error if we access the connection to early
            this.validateAndRefreshMetrics();
        }
    }

    public setFilterSourceWithPipeline(source: string, fromPipelineStepId: string, toPipelineStepId: string): void {
        this.internalState.source = source;
        this.internalState.fromPipelineStepId = fromPipelineStepId;
        this.internalState.toPipelineStepId = toPipelineStepId;
        this.internalState.pipelineFilterRelation = 'and';
        this.internalState.receiverId = null;
        this.validateAndRefreshMetrics();
    }

    public setFilterPipelineStep(pipelineStepId: string): void {
        this.internalState.source = null;
        this.internalState.fromPipelineStepId = pipelineStepId;
        this.internalState.toPipelineStepId = pipelineStepId;
        this.internalState.pipelineFilterRelation = 'or';
        this.internalState.receiverId = null;
        this.validateAndRefreshMetrics();
    }

    public setFilterReceiverId(receiverId: string): void {
        this.internalState.receiverId = receiverId;
        this.validateAndRefreshMetrics();
    }

    public clearFilter(): void {
        // we do not reset the beamtime id
        this.internalState.source = null;
        this.internalState.fromPipelineStepId = null;
        this.internalState.toPipelineStepId = null;
        this.internalState.receiverId = null;
        this.validateAndRefreshMetrics();
    }

    private validateAndRefreshMetrics(): void {
        if (this.isValidFilterOrSetError()) {
            connection.triggerMetricsRefresh(false /* do not reevaluate time*/);
        }
    }

    public isValidFilterOrSetError(): boolean {
        errorStore.clearFilterErrorText();
        if (!this.state.beamtime) {
            errorStore.setFilterErrorText('Beamtime must be set');
            return false;
        }
        return true;
    }
}

export const selectionFilterStore = new SelectionFilterStore();

(function loadLastBeamtimeFromStorage() {
    const lastBeamtime = localStorage.getItem(localStorageLastBeamtimeKey);
    if (lastBeamtime) {
        selectionFilterStore.setFilterBeamtime(lastBeamtime, true);
    }
})();

