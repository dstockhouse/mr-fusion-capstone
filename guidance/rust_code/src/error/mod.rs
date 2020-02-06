use crate::states::States;

#[derive(PartialEq, Debug)]
pub enum Error {
    SubSystem,
    PathDoesNotExist,
    RobotNotOnMap,
}

impl Error {
    pub fn handle(&self) -> Result<States, Error> {
        unimplemented!();
    }
}
