const path = require('path');
const webpack = require('webpack');

module.exports = {
    publicPath: '/tv/', // change it in src/router/index.js as well, I could not find how to do that automatically
    configureWebpack: {
        resolve: {
            alias: {
                flot: path.resolve(__dirname, './src/3dparty/flot'),
                flotPlugins: path.resolve(__dirname, './src/3dparty/flotPlugins'),
            }
        },
        plugins: [
            new webpack.ProvidePlugin({
                $: 'jquery',
                jQuery: 'jquery',
            }),
        ],
    },
    devServer: {
        proxy: {
            "^/tv-api": {
              target: "http://127.0.0.1:5020",
              changeOrigin: true,
              logLevel: "debug",
              pathRewrite: { "^/tv-api": "/" }
            }
        }
    }
};
