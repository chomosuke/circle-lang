use std::{fs, path::PathBuf};

use clap::Parser;

mod diagnostics;
mod lexer;

#[derive(Parser)]
struct Args {
    /// The source file to interpret.
    #[arg(value_name = "SOURCE FILE")]
    src_file: PathBuf,
}

fn main() {
    let Args { src_file } = Args::parse();
    let src =
        fs::read_to_string(src_file).unwrap_or_else(|e| panic!("Can not read source file: {e:?}"));
    lexer::process(&src);
}
