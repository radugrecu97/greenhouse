//Loading modules
var http = require('http');
var fs = require('fs');
var path = require('path');
var b = require('bonescript');
var child_process = require('child_process');
var exec = require('child_process').exec;

// Assign pin variables
var manual_heater = "P8_8";
var manual_tray = "P8_7";
var manual_led = "P8_9";
var manual_override = "P8_11";

// Initialize pins
b.pinMode(manual_heater, b.OUTPUT);
b.pinMode(manual_tray, b.OUTPUT);
b.pinMode(manual_led, b.OUTPUT);
b.pinMode(manual_override, b.OUTPUT);

// Initialize the server on port 8888
var server = http.createServer(function (req, res) {
    // requesting files
    var file = '.'+((req.url==='/')?'/index.html':req.url);
    var fileExtension = path.extname(file);
    var contentType = 'text/html';
    // Uncomment if you want to add css to your web page
    /*
    if(fileExtension === '.css'){
        contentType = 'text/css';
    }*/
    fs.exists(file, function(exists){
        if(exists){
            fs.readFile(file, function(error, content){
                if(!error){
                    // Page found, write content
                    res.writeHead(200,{'content-type':contentType});
                    res.end(content);
                }
            })
        }
        else{
            // Page not found
            res.writeHead(404);
            res.end('Page not found');
        }
    })
}).listen(8888);

// Loading socket io module
var io = require('socket.io').listen(server);

// When communication is established
io.on('connection', function (socket) {
    socket.on('change_heater', handle_change_heater);
    socket.on('change_tray', handle_change_tray);
    socket.on('change_override', handle_change_override);
    socket.on('led_intensity', handle_change_led);

    setInterval(function(){
        socket.emit('humidity', handle_humidity_update());
    }, 1000);
});

function handle_change_led(data) {
    let newData;
    try {
        newData = JSON.parse(data);
    }
    catch (e) {
        console.log("JSON parser error");
        return;
    }

    if (!Number.isInteger(newData.value)) {
        console.log("Not an integer");
        return;
    }

    if (!(newData.value >= 0 && newData.value <= 100)) {
        console.log("Integer not in range");
        return;
    }

    let led_intensity = newData.value;

    child_process.execSync(`./greenhouse_led ${led_intensity}`, (error, stdout, stderror) => {
        if (error) {
            console.error(`exec error: ${error}`);
            return;
        }
        console.log(`LED intensity set to: ${led_intensity}`);
    });
}

function handle_humidity_update() {
    let humidity = child_process.execSync('./Humidity19', (error, stdout, stderror) => {
        if (error) {
            console.error(`exec error: ${error}`);
            return;
        }
    });

    return humidity.toString();
}

// Change heater state when a button is pressed
function handle_change_heater(data) {
    var newData = JSON.parse(data);
    b.digitalWrite(manual_heater, newData.state);
}

// Change tray state when a button is pressed
function handle_change_tray(data) {
    var newData = JSON.parse(data);
    b.digitalWrite(manual_tray, newData.state);
}

function handle_change_override(data) {
    var newData = JSON.parse(data);
    b.digitalWrite(manual_override, newData.state);
}