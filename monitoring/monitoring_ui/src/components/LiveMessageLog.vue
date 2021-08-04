<template>
    <div class="relative">
        <div class="absolute w-full h-full">
            <div class="font-mono" v-for="entry in logEntries" :key="entry.id">
                <p class="whitespace-pre" :style="{ backgroundColor: entry.color }">{{entry.time}} [{{entry.level}}]{{''.padEnd(5 - entry.level.length)}} {{entry.message}}</p>
            </div>
        </div>
        <div class="absolute w-full h-full flex justify-center items-center" style="background-color: rgba(0,0,0,0.4);">
            <p class="text-white text-3xl">TODO: ElasicSearch log entries</p>
        </div>
    </div>
</template>

<script lang="ts">
import { Options, Vue } from "vue-class-component";
import Moment from "moment";

export default class LiveMessageLog extends Vue {
    private logEntries: {
        id: string,
        time: string,
        level: string,
        message: string,
        color: string,
        }[] = [];

    public mounted() {
        this.logEntries.push({
            id: '121',
            time: Moment(1622636450*1000+154).format('HH:mm:ss.S'),
            level: 'INFO',
            color: '#4caf50',
            message: 'testMessage1',
        });
        this.logEntries.push({
            id: '121',
            time: Moment(1622636451*1000+15).format('HH:mm:ss.S'),
            level: 'WARN',
            color: '#ff7043',
            message: 'testMessage2'
        });
        this.logEntries.push({
            id: '121',
            time: Moment(1622636451*1000+600).format('HH:mm:ss.S'),
            level: 'ERROR',
            color: '#ff1919',
            message: 'testMessage3'
        });

        this.logEntries.push({
            id: '121',
            time: Moment(1622636452*1000+154).format('HH:mm:ss.S'),
            level: 'INFO',
            color: '#4caf50',
            message: 'testMessage4',
        });
        this.logEntries.push({
            id: '121',
            time: Moment(1622636453*1000+15).format('HH:mm:ss.S'),
            level: 'WARN',
            color: '#ff7043',
            message: 'testMessage5'
        });
        this.logEntries.push({
            id: '121',
            time: Moment(1622636454*1000+600).format('HH:mm:ss.S'),
            level: 'ERROR',
            color: '#ff1919',
            message: 'testMessage6'
        });
    }
}
</script>

<style scoped lang="scss">
</style>
