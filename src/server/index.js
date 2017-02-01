let ttn = require('ttn');
let accessKey = require('./keys').accessKey;
let appId = 'biketheftmakezurich'

console.log('access key: ' + accessKey);
let client = new ttn.Client('eu', appId, accessKey);

client.on('message', function(deviceId, message) {
  console.log(message.payload_raw.toString('ascii'));
});
