let ttn = require('ttn');
let accessKey = require('./keys').accessKey;
let appId = 'biketheftmakezurich';
let express = require('express');
let open = require('open');
let locationByDevice = {};

console.log('access key: ' + accessKey);
let client = new ttn.Client('eu', appId, accessKey);

client.on('message', (deviceId, message) => {
  let location = message.payload_fields.location
  console.log(`Received location from device ${deviceId}. Latitude: ${location.lat}, Longitude: ${location.lng}`);
  locationByDevice[deviceId] = location;
});

const port = 3000;
const app = express();
app.get('*', (req, res) => {
  res.send(locationByDevice);
});

app.post('shutdown/:id', (req, res) => {
  const deviceId = req.params.id;
  let response = i % 3? '0': '1';
  client.send(deviceId, Buffer.from(response, 'ascii'));
});

app.listen(port, function(err) {
  if (err) {
    console.log(err);
  } else {
    open(`http://localhost:${port}`);
  }
});