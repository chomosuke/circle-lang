pub struct Pos {
    pub line: usize,
    pub column: usize,
}

pub struct Info {
    pos: Pos,
}

pub enum Diagnostics {
    Error { info: Info, msg: String },
}

impl Info {
    fn new(pos: Pos) -> Self {
        Self { pos }
    }
}
