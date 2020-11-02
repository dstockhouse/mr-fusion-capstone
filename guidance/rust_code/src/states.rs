#[derive(PartialEq, Debug)]
pub enum States {
    Wait,
    PlanPath,
    Traverse,
    Shutdown,
}
