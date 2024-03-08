use crate::diagnostics::{self, Diagnostics};


pub enum TokenKind {
    OpenBracket,
    CloseBracket,
    Comment(String),
    OpenBracket2,
    CloseBracket2,
    Assign,
    Name(String),
}

pub struct Token {
    pub info: diagnostics::Info,
    pub kind: TokenKind,
}

pub fn process(src: &str) -> Result<Vec<Token>, Diagnostics> {
    let mut tokens = Vec::new();
    let mut i = 0;
    
    while let Some(c)= char_indices.next() {
        while (i, c) = char_indices.next()
    }
    Ok(tokens)
}
