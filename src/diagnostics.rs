#[derive(Clone, Copy, Debug)]
pub struct Pos {
    pub line: usize,
    pub column: usize,
}

#[derive(Debug)]
pub struct Info {
    pub pos: Pos,
}

#[derive(Debug)]
pub enum Diagnostics {
    Error { info: Info, msg: String },
}
