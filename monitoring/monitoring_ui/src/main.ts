import { createApp, nextTick } from 'vue';
import App from './App.vue';;
import router from './router';

import './index.css';

import 'jquery';
import 'flot/jquery.flot';
import 'flot/jquery.flot.stack';
import 'flot/jquery.flot.crosshair';
import 'flotPlugins/jquery.flot.byte'

import moment from 'moment';

// Add and use a precise time format
// Otherwise it would show: 'a few seconds' instead of '3 seconds'
moment.defineLocale('precise-en', {
    relativeTime : {
        future : 'in %s',
        past : '%s ago',
        s : '%d seconds',
        ss : '%d seconds',
        m : '%d minute',
        mm : '%d minutes',
        h : '%d hour',
        hh : '%d hours',
        d : '%d day',
        dd : '%d days',
        M : '%d month',
        MM : '%d months',
        y : '%d year',
        yy : '%d years',
    },
});

createApp(App)
    .use(router)
    .mount('#app');


nextTick(() => {
    document.title = 'ASAPO Monitoring';
});