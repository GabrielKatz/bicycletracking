let accessKey = require('./keys').accessKey;
let appId = 'biketheftmakezurich';
let express = require('express');
let open = require('open');
let request = require('request');
let path = require('path');

const port = 3000;
const app = express();

let ttn = require('ttn');
let client = new ttn.Client('eu', appId, accessKey);

app.use(express.static('static'));
app.get('/coords/', (req, res) => {
  request(`https://${appId}.data.thethingsnetwork.org/api/v2/query?last=1d`, {
    method: "GET",
    headers: {
      'Authorization': 'key ' + accessKey
    }
  }, (error, response, body) => {
    if (!error && response.statusCode == 200) {

      let positionsByDevice = JSON.parse(body).filter(el => el.lat && el.lng).reduce((prev, message) => {
        let timeObject = {
          time: message.time,
          lat: message.lat,
          lng: message.lng
        }
        if(message.device_id in prev) {
          prev[message.device_id].push(timeObject);
        }
        else {
          prev[message.device_id] = [timeObject];
        }
        return prev;
      }, {});
      res.send(positionsByDevice);
    }
  });
});
app.get('/', (req,res ) => {
  res.sendFile(path.join( __dirname, '/static/biketracking.html'));
})


app.post('/shutdown/:id', (req, res) => {
  const deviceId = req.params.id;
  let response = '1';
  client.send(deviceId, Buffer.from(response, 'ascii'));
});

app.listen(port, function(err) {
  if (err) {
    console.log(err);
  } else {
    open(`http://localhost:${port}`);
  }
});