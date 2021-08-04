import { reactive } from "vue";
import { FixedTimeRange, TimeRange } from "../lib/TimeRange";
import { errorStore } from "./errorStore";

export interface TimeStoreState {
    refreshIntervalInSec: number | null;
    
    timeExpressionFrom: string;
    timeExpressionTo: string;

    lastFixedTimeRange: FixedTimeRange;
}

class TimeStore {
    private internalState: TimeStoreState = reactive({
        refreshIntervalInSec: 5,

        timeExpressionFrom: 'now-10m',
        timeExpressionTo: 'now',

        lastFixedTimeRange: {
            fromUnixSec: 0,
            toUnixSec: 0,
        }
    });

    public get state(): Readonly<TimeStoreState> {
        return this.internalState;
    }

    public setRefreshInterval(intervalInSec: number | null): void {
        this.internalState.refreshIntervalInSec = intervalInSec;
    }

    public setTimeRange(from: string, to: string): void {
        this.internalState.timeExpressionFrom = from;
        this.internalState.timeExpressionTo = to;
    }

    public evaluateAndVerify(): boolean {
        const range = new TimeRange(this.state.timeExpressionFrom, this.state.timeExpressionTo);
        errorStore.clearTimeErrorText();
        const evaled = range.evaluate();

        if (isNaN(evaled.fromUnixSec) || isNaN(evaled.toUnixSec)) {
            errorStore.setTimeErrorText('Invalid time format');
            return false;
        }

        this.internalState.lastFixedTimeRange.fromUnixSec = evaled.fromUnixSec;
        this.internalState.lastFixedTimeRange.toUnixSec = evaled.toUnixSec;
        return true;
    }
}

export const timeStore = new TimeStore();
