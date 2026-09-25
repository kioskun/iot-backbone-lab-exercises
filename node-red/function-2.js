if (msg.payload < 2000) {
    msg.payload = "low";
} else {
    msg.payload = "high";
}
return msg;
