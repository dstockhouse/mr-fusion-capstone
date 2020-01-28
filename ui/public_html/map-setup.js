var map = L.map('mapid', {
  center: [396, 306],
  crs: L.CRS.Simple,
  zoom: 3
});
map.fitBounds([
  [0, 0],
  [612, 792]
]);
var bgLayer = L.imageOverlay('./campus-map.png', [
  [0, 0],
  [612, 792]
]).addTo(map);

/* Attempt not to use jQuery
var marker;
map.on('click', function(event) {
  jQuery.post(
    'https://cs317-map-project.glitch.me/coords',
    { lat: event.latlng.lat, lng: event.latlng.lng },
    function(data) {
      if (!data) return;
      console.log(data);
      var popup = L.popup()
        .setLatLng(event.latlng)
        .setContent(
          `<h3>${data.name}</h3><p>${data.text}</p><p>Classes: ${data.class_name || ''}${
            data.class_time ? ': ' + data.class_time : ''
          }</p><p>Events: ${data.event_name || ''}${
            data.event_time ? ': ' + data.event_time : ''
          }</p>`
        )
        .openOn(map);
    }
  );
});
*/

console.log(
  jQuery.get('http://mercury.pr.erau.edu/~stockhod/mr-fusion/map.json', function(data) {
    console.log(data);
    L.geoJSON(data).addTo(map);
  })
);

