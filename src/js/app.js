//
// Pebble Watchface Steffen1
//
// (C) 2016 Steffen Liebergeld <perl@gmx.org>
//
// This program is licensed under Gnu General Public License, version 2. See
// LICENSE for details. I do not permit military use.
//
var xhrRequest = function (url, type, callback) {
    var xhr = new XMLHttpRequest();
    xhr.onload = function () {
      callback(this.responseText);
    };
    xhr.open(type, url);
    xhr.send();
};

var apikey='getyourown';
var url='http://api.openweathermap.org/data/2.5/forecast/daily?q=Dresden,DE&units=metric&lang=de&mode=json&cnt=2&APPID=' + apikey;

var getWeather = function () {
  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET',
      function(responseText) {
        var json = JSON.parse(responseText);

        console.log(responseText);
        console.log(json);

        var weather_dict = {
          'WEATHER_TEMP': json.list[0].temp.day,
          'WEATHER_COND': json.list[0].weather[0].description,
          'WEATHER_RAIN': json.list[0].rain,
          'WEATHER_TEMP_TOM': json.list[1].temp.day,
          'WEATHER_COND_TOM': json.list[1].weather[0].description,
          'WEATHER_RAIN_TOM': json.list[1].rain
        };

        Pebble.sendAppMessage(weather_dict,
            function(e) {
              console.log('Weather info sent to Pebble successfully!');
            },
            function(e) {
              console.log('Error sending weather info to Pebble!');
            });
      });
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log('PebbleKit JS ready!');
    getWeather();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log('AppMessage received!');
    getWeather();
  }
);

