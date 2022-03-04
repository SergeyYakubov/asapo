import { createRouter, createWebHistory, RouteRecordRaw } from "vue-router";
import Dashboard from "../views/Dashboard.vue";

const publicPath = "/tv"

// Possible lazy load with: component: () => import("../views/SomePage.vue"),
const routes: Array<RouteRecordRaw> = [
    {
        path: publicPath,
        name: "Dashboard",
        component: Dashboard,
    }
];

const router = createRouter({
    history: createWebHistory(),
    routes,
});

export default router;
