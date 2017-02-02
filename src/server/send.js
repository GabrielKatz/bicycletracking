let ttn = require('ttn');
let accessKey = require('./keys').accessKey;
let appId = 'biketheftmakezurich'

let client = new ttn.Client('eu', appId, accessKey);
let i = 0;
client.on('message', function(deviceId, message) {
    i++;
    console.log(`sending message to ${deviceId}`);
    let response = i % 3? '0': '1';
    client.send(deviceId, Buffer.from(response, 'ascii'));
});
