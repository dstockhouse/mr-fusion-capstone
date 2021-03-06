use crate::states::States;

#[derive(PartialEq, Debug)]
pub enum Error {
    WaitingSubSystem,
    PathPlanningPathDoesNotExist,
    PathPlanningNotOnMap,
    PathPlanningEdgeIndexNotInConnectionMatrix,
}

impl Error {
    pub fn handle(&self) -> Result<States, Error> {
        unimplemented!();
    }
}
