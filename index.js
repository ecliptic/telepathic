/* Expose the JS interface to the Node module */
var Client = require('./src/ClientJs.bs')

exports.start = Client.start
