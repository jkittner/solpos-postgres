#include "postgres.h"
#include "solpos00.h"
#include "utils/timestamp.h"

PG_MODULE_MAGIC;

/* handle an error within the solpos part and provide a useful error message to the
 * user.
 */
void
handle_error(const long code, struct posdata *pdat, TimestampTz ts)
{
    const char *ts_str = timestamptz_to_str(ts);

    if (code & (1L << S_YEAR_ERROR)) {
        elog(ERROR, "%s: Invalid year: %d (allowed range: [1950-2050])", ts_str,
             pdat->year);
    }
    else if (code & (1L << S_MONTH_ERROR)) {
        elog(ERROR, "%s: Invalid month: %d", ts_str, pdat->month);
    }
    else if (code & (1L << S_DAY_ERROR)) {
        elog(ERROR, "%s: Invalid day-of-month: %d", ts_str, pdat->day);
    }
    else if (code & (1L << S_HOUR_ERROR)) {
        elog(ERROR, "%s: Invalid hour: %d", ts_str, pdat->hour);
    }
    else if (code & (1L << S_MINUTE_ERROR)) {
        elog(ERROR, "%s: Invalid minute: %d", ts_str, pdat->minute);
    }
    else if (code & (1L << S_SECOND_ERROR))
        elog(ERROR, "%s: Invalid second: %d", ts_str, pdat->second);
    else if (code & (1L << S_TZONE_ERROR)) {
        elog(ERROR, "%s: Invalid time zone: %f", ts_str, pdat->timezone);
    }
    else if (code & (1L << S_INTRVL_ERROR)) {
        elog(ERROR, "Invalid data_interval: %d (allowed range: [0 - 28800])",
             pdat->interval);
    }
    else if (code & (1L << S_LAT_ERROR)) {
        elog(ERROR, "Invalid latitude: %f", pdat->latitude);
    }
    else if (code & (1L << S_LON_ERROR)) {
        elog(ERROR, "Invalid longitude: %f", pdat->longitude);
    }
    else {
        elog(ERROR,

             "An unknown error occurred while calling: solar_time(%s, %f, %f, %d)",
             ts_str, pdat->latitude, pdat->longitude, pdat->interval);
    }
}

PG_FUNCTION_INFO_V1(solar_time);

Datum
solar_time(PG_FUNCTION_ARGS)
{
    TimestampTz ts = PG_GETARG_TIMESTAMPTZ(0);
    /* this takes the timestamp into account making it a UNIX timestamp in UTC */
    pg_time_t orig_time_t = timestamptz_to_time_t(ts);
    float8 latitude = PG_GETARG_FLOAT8(1);
    float8 longitude = PG_GETARG_FLOAT8(2);
    int64 data_interval = PG_GETARG_INT64(3);
    /* create a pg_tm struct so we can access the date and time components */
    struct pg_tm *tm_result;
    tm_result = pg_gmtime(&orig_time_t);

    /* setup the solpos calculations */
    struct posdata pd, *pdat;
    pdat = &pd;
    S_init(pdat);
    pdat->function &= (S_TST & ~S_DOY);
    /* see pgtime.h. Year is relative to 1900 and month has its origin at 0 */
    pdat->year = tm_result->tm_year + 1900;
    pdat->month = tm_result->tm_mon + 1;
    pdat->day = tm_result->tm_mday;
    pdat->hour = tm_result->tm_hour;
    pdat->minute = tm_result->tm_min;
    pdat->second = tm_result->tm_sec;
    pdat->interval = data_interval;
    /* this should always be 0 since we use pg_gmtime*/
    pdat->timezone = tm_result->tm_gmtoff;
    pdat->longitude = longitude;
    pdat->latitude = latitude;

    const long retval = S_solpos(pdat);

    if (retval != 0) {
        handle_error(retval, pdat, ts);
    }

    /* add the offset to the date and calculate the result */
    int64 offset = (int64)(pdat->tstfix * 60000);
    TimestampTz new_time = time_t_to_timestamptz(orig_time_t);
    new_time = TimestampTzPlusMilliseconds(new_time, offset);
    /* return this as a tz naive timestamp since it's solar time */
    PG_RETURN_TIMESTAMP(new_time);
}
