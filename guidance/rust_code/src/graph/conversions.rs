
use super::{GPSPointDeg, TangentialPoint, GPSPointRad};
use nalgebra::{Vector3, Matrix3};

// Distance from the earth relative to the equator (m)
const RO: f64 = 6378137.0;

// Ecentricity of the earth (unitless?)
const E: f64 = 0.0818;

// Front entrance of king
const ORIGIN: GPSPointDeg = GPSPointDeg {
    lat: 34.6147979_f64,
    long: -112.4509615_f64,
    height: 1582.3
};

pub trait IntoTangential {
    type Output;

    fn into_tangential(&self) -> Self::Output;
}

impl IntoTangential for GPSPointDeg {

    type Output = TangentialPoint;

    fn into_tangential(&self) -> TangentialPoint {
        
        let origin = ORIGIN.to_rad();

        // vector from center of the earth to an arbitrary point on the map
        let r_ek_e = self.to_rad().to_xyz();

        // vector from the center of the earth to the origin of our tangential frame
        let r_eb_e = origin.to_xyz();

        // Vector that goes from origin to an arbitrary point of the map.
        // Still being expressed in terms of a rectangular frame with its origin at the center of
        // the earth.
        let r_kb_e = r_eb_e - r_ek_e;

        // The transformation matrix that converts the basis or our coordinate system
        let c_e_k = Matrix3::new(
            -(self.to_rad().long.cos() * self.to_rad().lat.sin()),       -origin.long.sin(),   -(origin.lat.cos() * origin.long.cos()), 
            -(origin.lat.sin() * origin.long.sin()),                      origin.long.cos(),   -(origin.lat.cos() * origin.long.sin()),
              origin.lat.cos(),                                           0.0,                  -origin.lat.sin() 
        ).try_inverse().unwrap();

        let result = c_e_k * r_kb_e;

        TangentialPoint{
            x: result[0],
            y: result[1],
            z: result[2]
        }
    }
}

impl GPSPointDeg {
    /// Converts the GPS from degrees to radians
    fn to_rad(&self) -> GPSPointRad {
        GPSPointRad {
            lat: self.lat.to_radians(),
            long: self.long.to_radians(),
            height: self.height
        }
    }
}

impl GPSPointRad {
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
}