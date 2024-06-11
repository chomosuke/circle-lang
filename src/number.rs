use std::collections::HashMap;

use num_bigint::BigInt;

#[derive(Debug, PartialEq, Eq)]
struct PIPoly(HashMap<BigInt, BigInt>);

#[derive(Debug, PartialEq, Eq)]
pub struct Number {
    numer: PIPoly,
    denom: PIPoly,
}

impl Number {
    pub fn from_name(name: String) -> Result<Self, String> {

    }

    pub fn from_string(str: String) -> Result<Self, String> {
        let err_f = |_| format!("{str} is not a valid number literal");
        let (numer, denom) = if let Some((pre, suf)) = str.split_once('.') {
            (
                format!("{pre}{suf}").parse().map_err(err_f)?,
                BigInt::from(10).pow(suf.len() as u32),
            )
        } else {
            (str.parse().map_err(err_f)?, 1.into())
        };
        Ok(Number {
            numer: PIPoly(HashMap::from([(1.into(), numer)])),
            denom: PIPoly(HashMap::from([(0.into(), denom)])),
        })
    }
}
