import moment from 'moment';
import * as relativeTimeParser from 'relative-time-parser';
import { errorStore } from '../store/errorStore';

/**
 * Contains a fixed unix time range
 */
export interface FixedTimeRange {
    fromUnixSec: number;
    toUnixSec: number;
}

/**
 * Contains a time range where the "from" and "to" values are just expressions
 * like from: "now-5m" to: "now"
 */
export class TimeRange {
    private static nowKeyword = 'now';

    public readonly from: string;
    public readonly to: string;

    public constructor(from: string | number, to: string | number) {
        this.from = String(from);
        this.to = String(to);
    }

    /**
     * Evaluates the time range expression
     * @returns {FixedTimeRange} A FixedUnix time that was evaluated by this expression
     */
    public evaluate(): FixedTimeRange {
        return {
            fromUnixSec: TimeRange.fromHumanExpressionToUnix(this.from),
            toUnixSec: TimeRange.fromHumanExpressionToUnix(this.to)
        };
    }

    /**
     * Evaluates and converts the current object to a human readable description.
     * e.g.
     *  {from: 'now-5m', to: 'now'} => 'Last 5 minutes'
     *  {from: '156423578', to: '159663578'} => '156423578 to 159663578'
     * @returns {String} The human readable description
     */
    public toHumanDescription(): string {
        const timeRange = this.evaluate();
        if (this.to === TimeRange.nowKeyword) {
            return `Last ${moment.duration(moment(timeRange.toUnixSec * 1000).diff(timeRange.fromUnixSec * 1000)).humanize()}`;
        }
        return this.from + ' to ' + this.to;
    }

    /**
     * Converts a relative time string to an unix timestamp
     * example: 'now-5s' => 1627403052
     * example: '-5s' => 1627403052
     * @param {string} input the expression
     * @returns {number} a unix time stamp
     */
    private static fromHumanExpressionToUnix(input: string): number {
        let realRelativeTime = input.replace(/\s/g, ''); // remove all spaces

        if (!relativeTimeParser().isRelativeTimeFormat(realRelativeTime)) {
            return Number(realRelativeTime);
        }

        const nowPosition = realRelativeTime.indexOf(TimeRange.nowKeyword);
        const backupRealRelativeTime = realRelativeTime;
        if (nowPosition === 0) { // The expression starts with "now"
            realRelativeTime = realRelativeTime.substring(nowPosition + TimeRange.nowKeyword.length);
        }

        if (realRelativeTime === '') { // there is nothing after now
            realRelativeTime = backupRealRelativeTime;
        }

        const resultAsMomentObject = relativeTimeParser().relativeTime(realRelativeTime);
        return resultAsMomentObject.unix();
    }
}
