#! errors.demo

variant ConfigError {
    Empty,
    Malformed(string),
}

@extern fn log_error(message: str);

fn parse_config(source: str) Result<i32, ConfigError> {
    if source == "" {
        return Result.Err(ConfigError.Empty);
    }
    let value = lookup_value(source)?;
    if value < 0 {
        return Result.Err(ConfigError.Malformed("negative"));
    }
    return Result.Ok(value);
}

fn unwrap_or(value: Result<i32, ConfigError>, fallback: i32) i32 {
    return switch (value) {
        Ok(v)   => v,
        Err(e)  => {
            log_error(describe(e));
            fallback
        },
    };
}

fn describe(error: ConfigError) str {
    return switch (error) {
        Empty             => "empty",
        Malformed(detail) => detail,
    };
}
