<?php
header("Access-Control-Allow-Origin: *");
header("Content-Type: application/json");

// Inputs are arrays of 2 coordinates as doubles
function latLonDist($point1, $point2) {
    // From https://www.movable-type.co.uk/scripts/latlong.html
    $R = 6371000;

    $lat1 = deg2rad($point1[0]);
    $lat2 = deg2rad($point2[0]);
    $lon1 = deg2rad($point1[1]);
    $lon2 = deg2rad($point2[1]);

    $a = pow(sin(($lat2 - $lat1)/2),2) + cos($lat1)*cos($lat2) * pow(sin(($lon2 - $lon1)/2),2);
    $c = 2*atan2(sqrt($a),sqrt(1-$a));

    return $R * $c;
}

function lat2x($lat) {
    return ($lat - 34.614714) * 61.38 / 0.001231 + 213.5;
}

function lon2y($lon) {
    return ($lon + 112.450971) * 85.53 / 0.002074 + 293.38;
}

function init_geojson_string() {
    return array("type"=>"FeatureCollection", "features"=>array());
}

$image_pixel_coordinates = 0;

class Node {
    private $name;
    private $coords;

    function __construct($name, $coord) {

        $this->name = $name;

        if(count($coord) == 2 &&
                gettype($coord[0]) == "double" &&
                gettype($coord[1]) == "double") {
            $this->coords = $coord;
        } else {
            $this->coords = array(0.0, 0.0);
        }
    }

    function get_name() {
        return $this->name;
    }

    function get_coords() {
        return $this->coords;
    }

    function to_geojson_feature() {
        global $image_pixel_coordinates;
        return array(
                "type"=>"Feature",
                "geometry"=>array(
                    "type"=>"Point",
                    "coordinates"=>array(
                        $image_pixel_coordinates ?
                            lon2y(floatval($this->coords[0])) : floatval($this->coords[0]),
                        $image_pixel_coordinates ?
                            lat2x(floatval($this->coords[1])) : floatval($this->coords[1]))),
                "id"=>$this->name,
                "properties"=>array("name"=>$this->name));
    }
}

class Edge {
    private $name;
    private $coords_array;
    private $length;

    function __construct($name, $coords) {

        $this->name = $name;

        $this->coords_array = array();
        if(count($coords) >= 2) {

            // Ensure valid coords and add to list
            foreach($coords as $coord) {
                if($coord && count($coord) == 2 &&
                        gettype($coord[0]) == "double" &&
                        gettype($coord[1]) == "double") {
                    array_push($this->coords_array, $coord);
                }
            }

            // Compute length
            $len = 0.0;
            for($i = 1; $i < count($this->coords_array); $i++) {
                $len += latLonDist($this->coords_array[$i-1],$this->coords_array[$i]);
            }
            $this->length = $len;
        } else {

            $this->coords_array = array(array(0.0, 0.0), array(0.0, 0.0));
            $this->length = 0.0;
        }
    }

    function get_name() {
        return $this->name;
    }

    function get_coords() {
        return $this->coords_array;
    }

    function get_length() {
        return $this->length;
    }

    function to_geojson_feature() {
        global $image_pixel_coordinates;
        $pix_coords = array();
        foreach($this->coords_array as $coord) {
            array_push($pix_coords, array(
                        $image_pixel_coordinates ?
                            lon2y(floatval($coord[0])) : floatval($coord[0]),
                        $image_pixel_coordinates ?
                            lat2x(floatval($coord[1])) : floatval($coord[1])));
        }
        return array(
                "type"=>"Feature",
                "geometry"=>array(
                    "type"=>"LineString",
                    "coordinates"=>$pix_coords),
                "id"=>$this->name,
                "properties"=>array("name"=>$this->name));
    }
}

class Graph {
    private $nodes;
    private $connections;

    public function get_closest_node_index($coords) {
        // echo "\n\tFinding closest node to (" .
            $coords[0] . "," . $coords[1] . ")\n";

        // Determine closest node based on coordinates
        $min_index = 0;
        $min_dist = INF;
        for($i = 0; $i < count($this->nodes); $i++) {

            $node_point = $this->nodes[$i]->get_coords();
            // echo "\t\ttesting node at (" .
                $node_point[0] . "," . $node_point[1] . ")\n";

            // distance to node
            $dist = latLonDist($coords, $node_point);
            // echo "\t\tDistance is " . $dist;
            if($dist < $min_dist) {
                // echo "\t\tselecting minimum index " . $i . "\n";
                $min_index = $i;
                $min_dist = $dist;
            }
        }

        // If closer than threshold, return index
        // echo "\t\tEnd: min dist is " . $min_dist . "\n";
        $dist_thresh = 5.0;
        if($min_dist < $dist_thresh) {
            return $min_index;
        } else {
            return -1;
        }
    }

    // Arguments are an array of nodes and an array of edges
    public function __construct($points, $lines) {

        // Add nodes
        $nodes_added = 0;
        $this->nodes = array();
        foreach($points as $point) {
            array_push($this->nodes, $point);
            $nodes_added++;
        }
        // echo "Added " . $nodes_added . " nodes\n";

        // Add edges and connections to nodes
        $this->connections = array();
        foreach($lines as $line) {

            $edge_coords = $line->get_coords();
            $first_point = $edge_coords[0];
            $last_point = $edge_coords[count($edge_coords)-1];

            // echo "\nstart (" . $first_point[0] . "," . $first_point[1] . ")\n";
            // echo "\nend (" . $last_point[0] . "," . $last_point[1] . ")\n";

            // Determine start and end nodes based on coordinates
            $start_node = $this->get_closest_node_index($first_point);
            $end_node = $this->get_closest_node_index($last_point);

            // If node found, add to connections
            if($start_node >= 0 && $end_node >= 0) {
                $this->connections[$start_node][$end_node] =
                    $this->connections[$end_node][$start_node] = $line;
            }

            /*
            // Print results for this edge
            echo "Start node (" . $this->nodes[$start_node]->get_coords()[0] . "," . $this->nodes[$start_node]->get_coords()[1] . ")\n";
            echo "\tstart min at " . $start_node . "\n";
            echo "End node (" . $this->nodes[$end_node]->get_coords()[0] . "," . $this->nodes[$end_node]->get_coords()[1] . ")\n";
            echo "\tend min at " . $end_node . "\n";
             */

        }
    }

    // Arguments are 2 arrays of lat lon (2 elements)
    public function find_json_path($start_coords, $end_coords) {

        // echo "Finding path\n";

        // Find nodes at start and end
        $start_node = $this->get_closest_node_index($start_coords);
        $end_node = $this->get_closest_node_index($end_coords);
        // echo "start node found: (" . $start_node . ")\n";
        // echo "end node found: (" . $end_node . ")\n";

        if($start_node < 0 || $end_node < 0) {
            // echo "Failed to find start and end nodes\n";
            return "";
        }

        /** Dijsktra's algorithm **/

        // Populate unvisited list
        $unvisited_dist = array($start_node=>0.0);
        for($i = 0; $i < count($this->nodes); $i++) {
            if($i != $start_node) {
                // Unvisited nodes have distance infinity
                $unvisited_dist[$i] = INF;
            }
        }

        $previous = array();
        do {
            // Current node is the node with shortest distance
            $cur_dist = INF;
            foreach($unvisited_dist as $node => $dist) {
                // echo "\tNode " . $node . " distance " . $dist . "\n";
                if($dist < $cur_dist) {
                    $current = $node;
                    $cur_dist = $dist;
                }
            }
            // echo "\t\tCurrent node " . $current . " distance " . $cur_dist . "\n";
            if($cur_dist == INF) {
                // No path was found
                // echo "No path found\n";
                return "";
            }

            // Update dist to all unvisited neighbors
            foreach($this->connections[$current] as $other => $edge) {
                // If other node is unvisited
                if(array_key_exists($other, $unvisited_dist)) {
                    $extra_dist = $edge->get_length();
                    $new_dist = $cur_dist + $extra_dist;

                    // If distance to other through current is less
                    // than previously calculated distance, update it
                    if($new_dist < $unvisited_dist[$other]) {
                        $unvisited_dist[$other] = $new_dist;
                        $previous[$other] = $current;
                    }

                }
            }
            // Remove current from unvisited list
            unset($unvisited_dist[$current]);

        } while($current != $end_node && count($unvisited_dist) > 0);


        // Now we have a path, traverse in reverse order to generate GeoJSON
        $json_out = init_geojson_string();
        array_push($json_out["features"],
                $this->nodes[$start_node]->to_geojson_feature());

        $current = $end_node;
        while($current != $start_node) {
            // Generate geojson for that node
            // array_push($json_out["features"],
                    // $this->nodes[$current]->to_geojson_feature());

            // Go back one node and generate the edge
            // If it was visited and has a connection to the previous (it should)
            if($previous[$current] && $this->connections[$current][$previous[$current]]) {
                array_push($json_out["features"],
                        $this->connections[$current][$previous[$current]]->to_geojson_feature());
                $current = $previous[$current];
            }
        }
        // Missed the start node, so generate geojson for start too
        array_push($json_out["features"],
                $this->nodes[$end_node]->to_geojson_feature());

        return $json_out;
    }
}


/**** START EXECUTION ****/

// Get HTTP request arguments
$pathsource = $_REQUEST["s"];
$pathdest = $_REQUEST["d"];

// $pathsource = "-112.4509615,34.6147979";
// $pathdest = "-112.4489055,34.6159122";
// $pathdest = "-112.45038986206056,34.61661888705339";

if ($pathsource != "" && $pathdest != "") {

    // Retrieve lat and lon from HTTP request
    sscanf($pathsource, "%f,%f", $sourcelon, $sourcelat);
    sscanf($pathdest, "%f,%f", $destlon, $destlat);

    // Setup output geojson
    $map_json = init_geojson_string();

    // Read KML (XML) file
    $map_filename = "map.kml";
    $map_xml = simplexml_load_file($map_filename) or die("Failed to " . $map_filename);

    // Loop through KML map and find nodes an edges for graph
    $g_nodes = array();
    $g_edges = array();
    foreach($map_xml->Document->Folder->Placemark as $pm) {

        // Print out Placemark information
        if($pm->Point) {

            // Split into coordinate array
            $coords_str = trim($pm->Point->coordinates);
            $coords = explode(",", $coords_str);
            $json_coords = array(
                $image_pixel_coordinates ?
                lon2y(floatval($coords[0])) : floatval($coords[0]),
                $image_pixel_coordinates ?
                lat2x(floatval($coords[1])) : floatval($coords[1]));

            // Add to nodes array
            array_push($g_nodes, new Node($pm->name, $json_coords));

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
            // Add to edges array
            array_push($g_edges, new Edge($pm->name, $json_coords));


            // Print lengths
        /*
           for($i = 1; $i < count($json_coords); $i++) {
           echo $json_coords[$i-1][0] . ", " . $json_coords[$i-1][1] . " to ";
           echo $json_coords[$i][0] . ", " . $json_coords[$i][1] . "\n\t";
           echo latLonDist($json_coords[$i-1], $json_coords[$i]) . "\n\n";
           }
         */

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

    // Generate graph
    $map_graph = new Graph($g_nodes, $g_edges);

    // Find path through map
    $path_json = $map_graph->find_json_path(
        array($sourcelon, $sourcelat),
        array($destlon, $destlat));

    echo json_encode($path_json);

    // echo json_encode($map_json);

} // else {

?>
