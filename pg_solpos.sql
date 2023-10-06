CREATE FUNCTION solar_time(ts timestamptz, lat double precision, lon double precision) RETURNS timestamptz
       AS '/usr/local/lib/funcs/pg_solpos.so', 'solar_time'
       LANGUAGE C STRICT;
