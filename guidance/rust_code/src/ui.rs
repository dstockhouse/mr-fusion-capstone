use lazy_static::*;
use cfg_if::cfg_if;

use std::fs::File;
use std::sync::Mutex;

pub const PATH_TO_UI_PIPE: &str = "to_ui";
pub const PATH_FROM_UI_PIPE: &str = "from_ui";

cfg_if! {
    if #[cfg(test)] {
        use mockall::*;
        use std::io;
        use std::path::Path;
        // If we are running a unit test, we don't want to actually open the pipe, since it blocks,
        // so let mock it out.
        mock! {
            pub OpenOptions {
                fn open<P: AsRef<Path> + 'static>(&self, path: P) -> io::Result<File>;
                fn append(&mut self, append: bool) -> &mut OpenOptions;
                fn create_new(&mut self, create_new: bool) -> &mut OpenOptions;
                fn read(&mut self, read: bool) -> &mut OpenOptions;
            }
        }

        use MockOpenOptions as OpenOptions;
    }
    else {
        use std::fs::OpenOptions;
    }
}

lazy_static! {
    // Opening the pipe with write permissions will block until the reader opens the file as well.
    pub static ref TO_UI: Mutex<File> = Mutex::new(OpenOptions::new()
        .append(true)
        .create_new(false)
        .open(PATH_TO_UI_PIPE)
        .unwrap() // Not sure how we want to handle an error here.
    );   
}

lazy_static! {
    pub static ref FROM_UI: Mutex<File> = Mutex::new(OpenOptions::new()
        .read(true)
        .open(PATH_FROM_UI_PIPE)
        .unwrap() // Not sure how we want to handle an error here.
    );
}





