use crate::{
    diagnostics::{self, Diagnostics, Pos},
    number::Number,
};

#[derive(Debug, PartialEq, Eq)]
pub enum TokenKind {
    OpenBracket,
    CloseBracket,
    Comment(String),
    OpenBracket2,
    CloseBracket2,
    Assign,
    Number(Number),
}

#[derive(Debug)]
pub struct Token {
    pub info: diagnostics::Info,
    pub kind: TokenKind,
}

enum PartialToken {
    OpenBracket(usize),
    CloseBracket(usize),
    Comment(String),
    Assign,
    Name(String),
    Number(String),
}

fn complete_token(partial: PartialToken, pos: Pos) -> Result<Token, Diagnostics> {
    let kind = match partial {
        PartialToken::OpenBracket(n) => {
            if n == 1 {
                TokenKind::OpenBracket
            } else if n == 2 {
                TokenKind::OpenBracket2
            } else {
                return Err(Diagnostics::Error {
                    info: diagnostics::Info { pos },
                    msg: format!(
                        r#"Found {n} consecutive "(".
If you intend to have multiple "(" or "((", please seperate them with whitespace."#
                    ),
                });
            }
        }
        PartialToken::CloseBracket(n) => {
            if n == 1 {
                TokenKind::CloseBracket
            } else if n == 2 {
                TokenKind::CloseBracket2
            } else {
                return Err(Diagnostics::Error {
                    info: diagnostics::Info { pos },
                    msg: format!(
                        r#"Found {n} consecutive ")".
If you intend to have multiple ")" or "))" please seperate them with whitespace."#
                    ),
                });
            }
        }
        PartialToken::Comment(content) => TokenKind::Comment(content),
        PartialToken::Assign => TokenKind::Assign,
        PartialToken::Name(str) => TokenKind::Number(Number::from_name(str)),
        PartialToken::Number(str) => TokenKind::Number(Number::from_string(str)),
    };
    Ok(Token {
        info: diagnostics::Info { pos },
        kind,
    })
}

fn start_token(c: char, pos: Pos) -> Result<Option<(PartialToken, Pos)>, Diagnostics> {
    Ok(if c.is_whitespace() {
        None
    } else {
        Some((
            match c {
                '#' => PartialToken::Comment(String::new()),
                '(' => PartialToken::OpenBracket(1),
                ')' => PartialToken::CloseBracket(1),
                '0'..='9' | '.' => PartialToken::Number(c.to_string()),
                ':' => PartialToken::Assign,
                'a'..='z' | 'A'..='Z' | '_' => PartialToken::Name(c.to_string()),
                _ => {
                    return Err(Diagnostics::Error {
                        info: diagnostics::Info { pos },
                        msg: format!("Unexpected token '{c}'"),
                    })
                }
            },
            pos,
        ))
    })
}

pub fn process(src: &str) -> Result<Vec<Token>, Diagnostics> {
    let mut tokens = Vec::new();
    let mut pos = Pos { line: 1, column: 1 };
    let mut partial_token_pos = None;
    for c in src.chars() {
        partial_token_pos = match partial_token_pos {
            None => start_token(c, pos)?,
            Some((partial_token, pos)) => match partial_token {
                PartialToken::Comment(mut content) => {
                    if c == '\n' {
                        tokens.push(complete_token(PartialToken::Comment(content), pos)?);
                        start_token(c, pos)?
                    } else {
                        content.push(c);
                        Some((PartialToken::Comment(content), pos))
                    }
                }
                PartialToken::Assign => {
                    if c == '=' {
                        tokens.push(complete_token(PartialToken::Assign, pos)?);
                        start_token(c, pos)?
                    } else {
                        return Err(Diagnostics::Error {
                            info: diagnostics::Info { pos },
                            msg: format!("Expected token '=', got '{c}'"),
                        });
                    }
                }
                PartialToken::Name(mut str) => {
                    if matches!(c, 'a'..='z' | 'A'..='Z' | '_' | '0' ..= '9') {
                        str.push(c);
                        Some((PartialToken::Name(str), pos))
                    } else {
                        tokens.push(complete_token(PartialToken::Name(str), pos)?);
                        start_token(c, pos)?
                    }
                }
                PartialToken::Number(mut str) => {
                    if matches!(c, '.' | '0'..='9') {
                        str.push(c);
                        Some((PartialToken::Name(str), pos))
                    } else {
                        tokens.push(complete_token(PartialToken::Number(str), pos)?);
                        start_token(c, pos)?
                    }
                }
                PartialToken::OpenBracket(n) => {
                    if c == '(' {
                        Some((PartialToken::OpenBracket(n + 1), pos))
                    } else {
                        tokens.push(complete_token(PartialToken::OpenBracket(n), pos)?);
                        start_token(c, pos)?
                    }
                }
                PartialToken::CloseBracket(n) => {
                    if c == ')' {
                        Some((PartialToken::CloseBracket(n + 1), pos))
                    } else {
                        tokens.push(complete_token(PartialToken::CloseBracket(n), pos)?);
                        start_token(c, pos)?
                    }
                }
            },
        };
        if '\n' == c {
            pos.line += 1;
            pos.column = 1;
        } else {
            pos.column += 1;
        }
    }
    Ok(tokens)
}

#[test]
fn test_lexer() {
    let tokens = process(
        r##"
(
    ((main))
    ((len)) := (
        ((len_))
        ((len_)) := 0
        ((len_temp))((0)) := ((len_in))((0))
        ((len_in))((0)) := magic_number
        # Remember the word magic_number will be transformed into a number
        ((len_out)) := 0
        (
            ((len_in))
        )
    )
    ((atoi)) := (
        ((atoi_))
        ((atoi_i)) := 0
        ((atoi_str))((atoi_i))
    )
    input
    ((atoi_str)) := ((input))
    ((atoi))
)"##,
    )
    .unwrap();
    let expected_tokens = [
        TokenKind::OpenBracket,
        TokenKind::OpenBracket2,
        TokenKind::Number(Number::from_name("ainm".to_owned())),
        TokenKind::CloseBracket2,
        TokenKind::OpenBracket2,
        TokenKind::Number(Number::from_name("enl".to_owned())),
        TokenKind::CloseBracket2,
        TokenKind::Assign,
        TokenKind::OpenBracket,
        TokenKind::OpenBracket2,
        TokenKind::Number(Number::from_name("en_l".to_owned())),
        TokenKind::CloseBracket2,
        TokenKind::OpenBracket2,
        TokenKind::Number(Number::from_name("en_l".to_owned())),
        TokenKind::CloseBracket2,
        TokenKind::Assign,
        TokenKind::Number(Number::from_string("0".to_owned())),
        TokenKind::OpenBracket2,
        TokenKind::Number(Number::from_name("en_templ".to_owned())),
        TokenKind::CloseBracket2,
        TokenKind::OpenBracket2,
        TokenKind::Number(Number::from_string("0".to_owned())),
        TokenKind::CloseBracket2,
        TokenKind::Assign,
        TokenKind::OpenBracket2,
        TokenKind::Number(Number::from_name("en_inl".to_owned())),
        TokenKind::CloseBracket2,
        TokenKind::OpenBracket2,
        TokenKind::Number(Number::from_string("0".to_owned())),
        TokenKind::CloseBracket2,
    ];
    assert_eq!(
        tokens
            .into_iter()
            .map(|token| token.kind)
            .collect::<Vec<_>>(),
        expected_tokens
    );
}
