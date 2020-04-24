<?php

header("Access-Control-Allow-Origin: *");
header("Content-Type: application/json");

function lat2x($lat) {
    return ($lat - 34.614714) * 61.38 / 0.001231 + 213.5;
}

function lon2y($lon) {
    return ($lon + 112.450971) * 85.53 / 0.002074 + 293.38;
}

function init_geojson_string() {
    return array("type"=>"FeatureCollection", "features"=>array());
}

/**** START EXECUTION ****/

$image_pixel_coordinates = 0;

// Setup output geojson
$map_json = init_geojson_string();

// Read KML (XML) file
$map_filename = "map.kml";
$map_xml = simplexml_load_file($map_filename) or die("Failed to " . $map_filename);

// Loop through KML map and find nodes an edges for graph
$g_nodes = array();
$g_edges = array();
foreach($map_xml->Document->Placemark as $pm) {

    // Print out Placemark information
    if($pm->Point) {

        // Split into coordinate array
        $coords_str = trim($pm->Point->coordinates);
        $coords = explode(",", $coords_str);
        $json_coords = array(
            $image_pixel_coordinates ?
                lon2y(floatval($coords[0])) : floatval($coords[0]),
            $image_pixel_coordinates ?
                lat2x(floatval($coords[1])) : floatval($coords[1])
        );


        // Add to JSON
        array_push($map_json["features"], array(
                    "type"=>"Feature", "geometry"=>array(
                        "type"=>"Point", "coordinates"=>$json_coords),
                    "id"=>$pm->name,
                    "properties"=>array("prop0"=>0)));

    } else if ($pm->LineString) {

        // Split into coordinate arrays
        $coords_strings = preg_split("/\s+/", $pm->LineString->coordinates);
        $json_coords = array();
        foreach($coords_strings as $coord) {
            if($coord) {
                $current = explode(",", $coord);
                array_push($json_coords, array(
                    $image_pixel_coordinates ?
                        lon2y(floatval($current[0])) : floatval($current[0]),
                    $image_pixel_coordinates ?
                        lat2x(floatval($current[1])) : floatval($current[1])
                    ));
            }
        }

        // Add to JSON
        array_push($map_json["features"], array(
                    "type"=>"Feature", "geometry"=>array(
                        "type"=>"LineString", "coordinates"=>$json_coords),
                    "id"=>$pm->name,
                    "properties"=>array("prop0"=>0)));

    } else {
        // echo " is unknown.";
    }

}

echo json_encode($map_json);

?>

