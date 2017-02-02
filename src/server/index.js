let ttn = require('ttn');
let accessKey = require('./keys').accessKey;
let appId = 'biketheftmakezurich'

let locationByDevice = {};

console.log('access key: ' + accessKey);
let client = new ttn.Client('eu', appId, accessKey);

client.on('message', function(deviceId, message) {

  let location = message.payload_fields.location
  console.log(`Received location from device ${deviceId}. Latitude: ${location.lat}, Longitude: ${location.lng}`);
  locationByDevice[deviceId] = location;
  
});
