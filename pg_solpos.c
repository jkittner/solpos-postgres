#include "postgres.h"
#include "fmgr.h"
#include "solpos00.h"
#include "utils/datetime.h"
#include "utils/timestamp.h"

PG_MODULE_MAGIC;

PG_FUNCTION_INFO_V1(solar_time);

Datum
solar_time(PG_FUNCTION_ARGS)
{
    pg_time_t orig_time_t = timestamptz_to_time_t(PG_GETARG_TIMESTAMPTZ(0));
    float8 latitude = PG_GETARG_FLOAT8(1);
    float8 longitude = PG_GETARG_FLOAT8(2);
    struct pg_tm *tm_result;
    tm_result = pg_localtime(&orig_time_t, log_timezone);

    struct posdata pd, *pdat;
    pdat = &pd;
    S_init(pdat);

    pdat->function &= (S_TST & ~S_DOY);
    pdat->year = tm_result->tm_year + 1900;
    pdat->month = tm_result->tm_mon + 1;
    pdat->day = tm_result->tm_mday;
    pdat->hour = tm_result->tm_hour;
    pdat->minute = tm_result->tm_min;
    pdat->second = tm_result->tm_sec;
    pdat->timezone = tm_result->tm_gmtoff;
    pdat->longitude = longitude;
    pdat->latitude = latitude;

    long retval = S_solpos(pdat);

    if (retval != 0) {
        elog(ERROR, "internal error in the solpos function");
    }

    int64 offset = (int64)(pdat->tstfix * 60000);
    TimestampTz new_time = time_t_to_timestamptz(orig_time_t);
    new_time = TimestampTzPlusMilliseconds(new_time, offset);
    PG_RETURN_TIMESTAMPTZ(new_time);
}
