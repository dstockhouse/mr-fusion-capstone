
function loadJSON(callback) {

    var xobj = new XMLHttpRequest();
    xobj.overrideMimeType("application/json");
    console.log('Starting HTTP GET');
    xobj.onreadystatechange = function () {
        if (xobj.readyState == 4 && xobj.status == "200") {
            // Required use of an anonymous callback as .open will NOT return a value but simply returns undefined in asynchronous mode
            console.log('Successful request');
            callback(xobj.responseText);
        }
    };
    xobj.open('GET', 'decode-map.php', true);
    // xobj.open('GET', 'map.json', true);
    xobj.send();
}

function fetchPath(latlng1, latlng2, callback) {

    var xobj = new XMLHttpRequest();
    xobj.overrideMimeType("application/json");
    console.log('Starting HTTP GET');
    xobj.onreadystatechange = function () {
        if (xobj.readyState == 4 && xobj.status == "200") {
            // Required use of an anonymous callback as .open will NOT return a value but simply returns undefined in asynchronous mode
            console.log('Successful request');
            callback(xobj.responseText);
        }
    };
    var requrl = 'path.php?s='+latlng1.lng.toString()+','+latlng1.lat.toString()+'&d='+latlng2.lng.toString()+','+latlng2.lat.toString();
    console.log('Requesting path from: ' + requrl);
    xobj.open('GET', requrl, true);
    // xobj.open('GET', 'map.json', true);
    xobj.send();
}

console.log('map init');

var mymap = L.map('mapid').setView([34.616, -112.449], 18);

L.tileLayer('https://api.tiles.mapbox.com/v4/{id}/{z}/{x}/{y}.png?access_token=pk.eyJ1IjoibWFwYm94IiwiYSI6ImNpejY4NXVycTA2emYycXBndHRqcmZ3N3gifQ.rJcFIG214AriISLbB6B5aw', {
    maxZoom: 18,
    attribution: 'Map data &copy; <a href="https://www.openstreetmap.org/">OpenStreetMap</a> contributors, ' +
    '<a href="https://creativecommons.org/licenses/by-sa/2.0/">CC-BY-SA</a>, ' +
    'Imagery © <a href="https://www.mapbox.com/">Mapbox</a>',
    id: 'mapbox.streets'
}).addTo(mymap);

// Handle click events
var clicknum = 0;
var pathsource;
var pathdest;
var pathjson;
var pathjsonlayer;
var popup1 = L.popup();
var popup2 = L.popup();
mymap.on('click', function(e) {

    console.log('Click ' + clicknum.toString() + ' at ' + e.latlng.toString());

    // Decide what to do based on how many clicks have happened
    if (clicknum == 0) {

        clicknum = 1;

        pathsource = e.latlng;
        popup1.setLatLng(e.latlng)
            .setContent('Source: ' + pathsource.lat.toString() + ' N, ' + pathsource.lng.toString() + ' E')
            .openOn(mymap);

    } else if (clicknum == 1) {

        clicknum = 2;

        pathdest = e.latlng;
        var contentstring = 'Dest: ' + pathdest.lat.toString() + ' N, ' + pathdest.lng.toString() + ' E';
        console.log(contentstring);
        popup2.setLatLng(pathdest)
            .setContent(contentstring)
            .openOn(mymap);

        // Plan path
        console.log('Fetching path');
        fetchPath(pathsource, pathdest, function(data) {
            console.log('loading path json');
            console.log(data);
            pathjson = JSON.parse(data);
            console.log('finished parsing path data');
            console.log(pathjson);


            mapjsonlayer.remove();
            pathjsonlayer = L.geoJSON(pathjson, { filter: function(feature, layer) {
                if (feature.id[0].includes("Point")) {
                    return false;
                } else {
                    return true;
                }
            }
            }).addTo(mymap);
        });
    } else {

        clicknum = 0;
        pathjsonlayer.remove();
        mapjsonlayer.addTo(mymap);
    }
});

// Fetch map JSON
var mapjson;
var mapjsonlayer;
console.log('fetch JSON');
loadJSON(function(data) {

    console.log('loading map json');
    mapjson = JSON.parse(data);
    console.log('finished parsing map data');
    console.log(mapjson);


    mapjsonlayer = L.geoJSON(mapjson, { filter: function(feature, layer) {
        if (feature.id[0].includes("Point")) {
            return false;
        } else {
            return true;
        }
    }
    }).addTo(mymap);
});


