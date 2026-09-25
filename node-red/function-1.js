let data = msg.payload.uplink_message.decoded_payload;

let temp = data.DHT_Temp;   // Temperature
let hum = data.DHT_Hum;    // Humidity
let photo = data.Photo;      // Photoresistor value (numeric)
let led = (photo < 2000) ? 1 : 0; // LED value (ensure it's a number)

return [
    { payload: temp },
    { payload: hum },
    { payload: photo },
    { payload: led }
];
