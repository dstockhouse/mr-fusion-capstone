#include <vector>

typedef struct {
    int latitude;
    int longitude;
} GPSPoint;

typedef struct {
    // For debugging, finding out what line we are looking at on the map
    char* name; 

    int numberOfPoints;
    double distance;
    std::vector<GPSPoint> gpsPoints;
} MapLine;

typedef struct mapPoint {
    // Will be used to display key locations to UI
    char* name;

    // Key pointer to determine the shortest path using Dijkstra's Algorithm
    struct mapPoint* parentPoint;

    // A map line that connects the point to an adjectent point will share the 
    // same index
    std::vector<mapPoint*> adjacentMapPoints;
    std::vector<MapLine*> mapLines;

    
} MapPoint;