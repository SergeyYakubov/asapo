module.exports = function (context, options) {
  return {
    name: 'custom-docusaurus-plugin',
    configureWebpack(config, isServer, utils) {
      const {getCacheLoader} = utils;
      return {
        module: {
          rules: [
            {
                test: /\.sh$/i,
                use: 'raw-loader',
            },
            {
              test: /\.cpp$/i,
              use: 'raw-loader',
            },
            {
              test: /\.c$/i,
              use: 'raw-loader',
            },
            {
              test: /Makefile/i,
              use: 'raw-loader',
            },
            {
              test: /\.txt$/i,
              use: 'raw-loader',
            },
            {
              test: /\.py$/i,
              use: 'raw-loader',
            },
          ],
        },
      };
    },
  };
};
