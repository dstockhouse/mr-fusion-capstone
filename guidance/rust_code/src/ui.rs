use lazy_static::*;
use std::fs;
use std::fs::File;
use std::sync::Mutex;

pub const PATH_TO_UI_PIPE: &str = "to_ui";
pub const PATH_FROM_UI_PIPE: &str = "from_ui";

// Mutexes are needed so the file can be mutable. Since we will always be using one thread, 
// acquiring the lock should never block.
lazy_static! {
    // Opening the pipe with write permissions will block until the reader opens the file as well.
    pub static ref TO_UI: Mutex<File> = Mutex::new(fs::OpenOptions::new()
        .append(true)
        .create_new(false)
        .open(PATH_TO_UI_PIPE)
        .unwrap()
    );   
}

lazy_static! {
    pub static ref FROM_UI: Mutex<File> = Mutex::new(fs::OpenOptions::new()
        .read(true)
        .open(PATH_FROM_UI_PIPE)
        .unwrap()
    );
}
