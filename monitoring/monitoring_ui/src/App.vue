<template>
    <router-view />
    <Tooltip />
</template>


<script lang="ts">
import { Options, Vue } from 'vue-class-component';
import Tooltip from './components/Tooltip.vue';

@Options({
    components: {
        Tooltip
    }
})
export default class App extends Vue {
}

</script>

<style lang="css">
/*
#app {
    font-family: Avenir, Helvetica, Arial, sans-serif;
    -webkit-font-smoothing: antialiased;
    -moz-osx-font-smoothing: grayscale;
    text-align: center;
    color: #2c3e50;
}

#nav {
    padding: 30px;

    a {
        font-weight: bold;
        color: #2c3e50;

        &.router-link-exact-active {
            color: #42b983;
        }
    }
}
*/
</style>
