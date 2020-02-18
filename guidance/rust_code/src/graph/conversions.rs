use std::f64::consts::PI;

use super::{GPSPoint, TangentialPoint, Graph, Edge, Vertex};
use nalgebra::{Vector3, Matrix3};

// Distance from the earth relative to the equator (m)
const RO: f64 = 6378137.0;

// Ecentricity of the earth (unitless?)
const E: f64 = 0.0818;

// Front entrance of king
const ORIGIN: GPSPoint = GPSPoint {
    lat: 34.6147979,
    long: -112.4509615,
    height: 1582.3
};

pub trait IntoTangential {
    type Output;

    fn into_tangential(self) -> Self::Output;
}

impl IntoTangential for GPSPoint {

    type Output = TangentialPoint;

    fn into_tangential(self) -> TangentialPoint {

        // vector from center of the earth to an arbitrary point on the map
        let r_ek_e = self.to_rad().to_xyz();

        // vector from the center of the earth to the origin of our tangential frame
        let r_eb_e = ORIGIN.to_rad().to_xyz();

        // Vector that goes from origin to an arbitrary point of the map.
        // Still being expressed in terms of a rectangular frame with its origin at the center of
        // the earth.
        let r_kb_e = r_eb_e - r_ek_e;

        // The transformation matrix that converts the basis or our coordinate system
        let c_e_k = Matrix3::new(
            -(self.long.cos() * self.lat.sin()),       -ORIGIN.long.sin(),   -(ORIGIN.lat.cos() * ORIGIN.long.cos()), // Check this line
            -(ORIGIN.lat.sin() * ORIGIN.long.sin()),    ORIGIN.long.cos(),   -(ORIGIN.lat.cos() * ORIGIN.long.sin()),
              ORIGIN.lat.cos(),                         0.0,                  -ORIGIN.lat.sin() 
        ).try_inverse().unwrap();

        let result = c_e_k * r_kb_e;

        TangentialPoint{
            x: result[0],
            y: result[1],
            z: result[2]
        }

    }
}

impl GPSPoint {

    /// Converts spherical cords to rectangular
    fn to_xyz(&self) -> Vector3<f64> {
        
        // The distance from the center of the earth relative to where you are (m)
        let re = RO / (1.0 - self.lat.sin().powi(2)*E.powi(2)).sqrt();

        Vector3::new(
            (re + self.height) * self.lat.cos() * self.long.cos(),
            (re + self.height) * self.lat.cos() * self.long.sin(),
            ((re + re*E.powi(2)) + self.height) * self.lat.sin()
        )
    }

    /// Converts the GPS from degrees to radians
    fn to_rad(&self) -> GPSPoint {
        GPSPoint {
            lat: self.lat.to_radians(),
            long: self.long.to_radians(),
            height: self.height
        }
    }
}

impl<'a> IntoTangential for Graph<'a, GPSPoint> {
    
    type Output = Graph<'a, TangentialPoint>;

    fn into_tangential(self) -> Self::Output {
        
        // Preserving the connection matrix
        let connection_matrix = self.connection_matrix;

        let edges = self.edges.into_iter().map(|edge| {
            let name = edge.name;
            let points = edge.points.into_iter().map(|gps_point| {
                gps_point.into_tangential()
            }).collect::<Vec<TangentialPoint>>();

            Edge{name, points}

        }).collect();

        let vertices = self.vertices.into_iter().map(|vertex| {
            let name = vertex.name;
            let point = vertex.point.into_tangential();

            Vertex::new(name, point)
        }).collect(); 

        Graph{connection_matrix, edges, vertices}
    }
}