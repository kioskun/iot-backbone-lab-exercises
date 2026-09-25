// TTN uplink payload formatter (Custom JavaScript formatter).
// Converts the received bytes to text and parses the JSON message.
function decodeUplink(input) {
  var bytes = input.bytes;
  var text = String.fromCharCode.apply(null, bytes);
  var data = {};
  try {
    data = JSON.parse(text);
  } catch (error) {
    data.raw = text;
  }
  return { data: data };
}
