const path = require('path');
const webpack = require('webpack');

module.exports = {
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
            "^/api": {
              target: "http://127.0.0.1:5020",
              changeOrigin: true,
              logLevel: "debug",
              pathRewrite: { "^/api": "/" }
            }
        }
    }
};
