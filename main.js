var mraa = require('mraa'); //require mraa
console.log('MRAA Version: ' + mraa.getVersion()); //write the mraa version to the Intel XDK console

//Sets up pins to communicate with Arduino
var one = new mraa.Gpio(10);
one.dir(mraa.DIR_OUT);
var two = new mraa.Gpio(11);
two.dir(mraa.DIR_OUT);
var three = new mraa.Gpio(12);
three.dir(mraa.DIR_OUT);
//Signal to let Arduino know when to test other pins
var signal = new mraa.Gpio(5);
signal.dir(mraa.DIR_OUT);

//Set up http for weather api
var http = require('http');
// city name
var city = "Detroit";
// API Key
var apiKey = "256bd905eee36f110f18f9598c25b2ad";

//get current time
var hour = new Date().getHours();
//convert from UTC to EST
hour = hour - 5;
if (hour < 0)
{
    hour = hour + 24;
}

//Set up blynk
var Blynk = require('blynk-library');
//authorization code for blynk
//var AUTH = 'f3967b2a26ed48ba94ff8f4bcf2d3ad6';
var AUTH = "5ff3e21dc43e48e28d1ac68f6453bba7";
var blynk = new Blynk.Blynk(AUTH, options = {
  connector : new Blynk.TcpClient()
});

//Match each button with appropriate pin
var weatherButton = new blynk.VirtualPin(1);
var blueSkyButton = new blynk.VirtualPin(2);
var nightButton = new blynk.VirtualPin(3);
var overcastButton = new blynk.VirtualPin(4);
var goldenButton = new blynk.VirtualPin(5);
var snowButton = new blynk.VirtualPin(6);
var lightningButton = new blynk.VirtualPin(7);
var rainbowButton = new blynk.VirtualPin(8);

weatherButton.on('write', function(param) {
    if (param[0] == 1)
    {
        getWeather();
    }
});

blueSkyButton.on('write', function(param) {
    if (param[0] == 1)
    {
        blueSky();
    }
});

nightButton.on('write', function(param) {
    if (param[0] == 1)
    {
        night();
    }
});

overcastButton.on('write', function(param) {
    if (param[0] == 1)
    {
        overcast();
    }
});

goldenButton.on('write', function(param) {
    if (param[0] == 1)
    {
        golden();
    }
});

snowButton.on('write', function(param) {
    if (param[0] == 1)
    {
        snow();
    }
});

lightningButton.on('write', function(param) {
    if (param[0] == 1)
    {
        lightning();
    }
});

rainbowButton.on('write', function(param) {
    if (param[0] == 1)
    {
        rainbow();
    }
});

function getWeather() {
    var url = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "?id=524901&APPID=" + apiKey; 
    
    try {
    
    // build the http request
    http.get(url, function(res) {
        var body = '';

        // read the response of the query
        res.on('data', function(chunk) {
        body += chunk;
        });

        res.on('end', function() {
        // now parse the json feed
        var weather = JSON.parse(body)

        // get the current weather condition
        var condition = weather.weather[0].main;
        
        if (hour < 7 || hour > 19)
        {
            night();
        }
        else if (condition == "Thunderstorm" || condition == "Rain" || condition == "Extreme")
        {
            lightning();
        }
        else if (condition == "Drizzle" || condition == "Atmosphere" || condition == "Clouds")
        {
            overcast();
        }
        else if (condition == "Snow")
        {
            snow();
        }
        else if (condition == "Clear")
        {
            golden();
        }
        else
        {
            blueSky();
        }
            
        });
    
    }).on('error', function(e) {
        
        // check for errors and eventually show a message
        console.log("Error");

    });
        
    } catch(e) {
        
        // check for errors and eventually show a message
        console.log("Error");

    }
    
};

function off(){
    one.write(0);
    two.write(0);
    three.write(0);
    signal.write(1);
}

function blueSky(){
    one.write(0);
    two.write(0);
    three.write(1);
    signal.write(1);
}

function night(){
  one.write(0);
  two.write(1);
  three.write(0);
  signal.write(1);
}

function overcast(){
  one.write(0);
  two.write(1);
  three.write(1);
  signal.write(1);
}

function golden(){
  one.write(1);
  two.write(0);
  three.write(0);
  signal.write(1);
}

function snow(){
  one.write(1);
  two.write(0);
  three.write(1);
  signal.write(1);
}

function lightning(){
  one.write(1);
  two.write(1);
  three.write(0);
  signal.write(1);
}

function rainbow(){
  one.write(1);
  two.write(1);
  three.write(1);
  signal.write(1);
}