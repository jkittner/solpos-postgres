CREATE FUNCTION solar_time(
    ts TIMESTAMPTZ,
    lat DOUBLE PRECISION,
    lon DOUBLE PRECISION,
    data_interval INT DEFAULT 0
) RETURNS TIMESTAMP WITHOUT TIME ZONE
AS '/usr/local/lib/funcs/pg_solpos.so', 'solar_time'
LANGUAGE C STRICT;
