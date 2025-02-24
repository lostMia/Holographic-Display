const copyPlugin = require('copy-webpack-plugin');
const path = require('path');

module.exports = {
  mode: 'development',
  entry: './src/main/script.js',
  output: {
    filename: 'main.js',
    path: path.resolve(__dirname, '../data/site/main/')
  },

  plugins: [
    new copyPlugin({
      patterns: [
        { from: 'src/main/', to: '../main/' },
        { from: 'src/notfound/', to: '../notfound/' },
        { from: 'src/resources/', to: '../resources/' },
      ],
    }),
  ],
};
