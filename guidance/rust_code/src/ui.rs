use lazy_static::*;
use cfg_if::cfg_if;

use std::sync::Mutex;

pub const PATH_TO_UI_PIPE: &str = "to_ui";
pub const PATH_FROM_UI_PIPE: &str = "from_ui";

cfg_if! {
    if #[cfg(test)] {
        use mockall::*;
        use std::io;
        use std::io::Write;

        mock! {
            pub File {}
            trait Write {
                fn write(&mut self, buf: &[u8]) -> io::Result<usize>;
                fn flush(&mut self) -> io::Result<()>;
            }
        }

        lazy_static! {
            // Opening the pipe with write permissions will block until the reader opens the file as well.
            pub static ref TO_UI: Mutex<MockFile> = Mutex::new(MockFile::new());   
        }

        lazy_static! {
            pub static ref FROM_UI: Mutex<MockFile> = Mutex::new(MockFile::new());
        }
    }
    else {
        use std::fs::{OpenOptions, File};

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
    }
}







