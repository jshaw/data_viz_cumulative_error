var SerialPort = require('serialport');
var PubNub = require('pubnub');

var port = new SerialPort('/dev/cu.usbmodem1451', {
	baudRate: 115200,
    parser: SerialPort.parsers.readline("\n")
}, function (err) {
	if (err) {
		return console.log('Error: ', err.message);
	}
	port.write('main screen turn on', function(err) {
		if (err) {
				return console.log('Error on write: ', err.message);
        }
        console.log('message written');
    });
    port.on('data', function(data) {
        console.log(data);
    });
});

function publish() {
   
    pubnub = new PubNub({
        publishKey : 'pub-c-48d8a716-ad50-444f-8e60-22978d42537f',
        subscribeKey : 'sub-c-e33c40c6-be23-11e6-9868-02ee2ddab7fe',
        secretKey: "sec-c-N2RiY2Q2NjgtOWU5NC00YWMzLWI3YTAtNjVkZWVlNWU4NDU2",
    })
       
    function publishSampleMessage() {
        console.log("Since we're publishing on subscribe connectEvent, we're sure we'll receive the following publish.");
        var publishConfig = {
            channel : "data_viz",
            message : "Hello from PubNub Docs!"
        }
        pubnub.publish(publishConfig, function(status, response) {
            console.log(status, response);
        })
    }
       
    pubnub.addListener({
        status: function(statusEvent) {
            if (statusEvent.category === "PNConnectedCategory") {
                publishSampleMessage();
            }
        },
        message: function(message) {
            console.log("New Message!!", message);
        },
        presence: function(presenceEvent) {
            // handle presence
        }
    })      
    console.log("Subscribing..");
    pubnub.subscribe({
        channels: ['data_viz'] 
    });
};

publish();