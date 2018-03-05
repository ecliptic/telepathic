var path = require('path')

module.exports = {
  entry: './src/ClientJs.bs.js',
  mode: 'production',
  output: {
    path: path.resolve(__dirname, 'dist'),
    filename: 'Telepathic.js',
    libraryTarget: 'commonjs',
  },
}
