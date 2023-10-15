from datetime import datetime

import psycopg.errors
import pytest


@pytest.fixture(scope='session')
def docker_compose_file() -> str:
    return 'docker-compose.yml'


@pytest.fixture(scope='session')
def postgresql(docker_ip, docker_services):
    def _test_postgresql(dsn):
        try:
            psycopg.connect(dsn)
        except psycopg.DatabaseError:
            return False
        return True

    port = docker_services.port_for('db', 5432)
    host = docker_ip
    dsn = f'postgresql://postgres:test@{host}:{port}/postgres'

    docker_services.wait_until_responsive(
        timeout=60,
        pause=1,
        check=lambda: _test_postgresql(dsn),
    )

    return dsn


@pytest.fixture(scope='session')
def db(postgresql, docker_services):
    with psycopg.connect(postgresql, autocommit=True) as conn:
        yield conn


@pytest.fixture(scope='session', autouse=True)
def initialize_sql_function(db: psycopg.Connection) -> None:
    with open('pg_solpos.sql') as f:
        sql = f.read()
    db.execute(sql)


def test_solar_time_date_tz_naive(db):
    with db.cursor() as cur:
        cur.execute("SELECT solar_time('2023-10-06 15:30'::TIMESTAMP, 51.481, 7.217)")  # noqa: E501
        d, = cur.fetchone()

    assert d == datetime(2023, 10, 6, 16, 10, 43, 700000)


def test_solar_time_date_tz_aware(db):
    with db.cursor() as cur:
        cur.execute("SELECT solar_time('2023-10-06 15:30+02:00'::TIMESTAMPTZ, 51.481, 7.217)")  # noqa: E501
        d, = cur.fetchone()

    assert d == datetime(2023, 10, 6, 14, 10, 42, 363000)


def test_solar_time_date(db):
    with db.cursor() as cur:
        cur.execute(
            "SELECT solar_time('2023-10-06 15:30'::DATE, 51.481, 7.217)",
        )
        d, = cur.fetchone()

    assert d == datetime(2023, 10, 6, 0, 40, 32, 94000)


def test_solar_time_lat_numeric(db):
    with db.cursor() as cur:
        cur.execute("SELECT solar_time('2023-10-06 15:30'::TIMESTAMP, 51.481::NUMERIC, 7.217)")  # noqa: E501
        d, = cur.fetchone()

    assert d == datetime(2023, 10, 6, 16, 10, 43, 700000)


def test_solar_time_lon_int(db):
    with db.cursor() as cur:
        cur.execute("SELECT solar_time('2023-10-06 15:30'::TIMESTAMP, 51.481, 7.217::INT)")  # noqa: E501
        d, = cur.fetchone()

    assert d == datetime(2023, 10, 6, 16, 9, 51, 621000)


def test_solar_time_lat_int(db):
    with db.cursor() as cur:
        cur.execute("SELECT solar_time('2023-10-06 15:30'::TIMESTAMP, 51.481::INT, 7.217)")  # noqa: E501
        d, = cur.fetchone()

    assert d == datetime(2023, 10, 6, 16, 10, 43, 700000)


def test_solar_time_different_timezone(db):
    # Australia has strange half hour TZs
    with db.cursor() as cur:
        cur.execute("SELECT solar_time('2023-10-06 15:30:00 ACST', -34.885, 138.579)")  # noqa: E501
        d, = cur.fetchone()

    assert d == datetime(2023, 10, 6, 15, 26, 3, 484000)


@pytest.mark.parametrize('lat', (-91, 91))
def test_solar_time_invalid_latitude(db, lat):
    with db.cursor() as cur:
        with pytest.raises(psycopg.errors.InternalError) as exc:
            cur.execute(f"SELECT solar_time('2023-10-06 15:30'::TIMESTAMP, {lat}, 7.217)")  # noqa: E501

    msg, = exc.value.args
    assert msg == f'Invalid latitude: {lat}.000000'


@pytest.mark.parametrize('lon', (-181, 181))
def test_solar_time_invalid_longitude(db, lon):
    with db.cursor() as cur:
        with pytest.raises(psycopg.errors.InternalError) as exc:
            cur.execute(f"SELECT solar_time('2023-10-06 15:30'::TIMESTAMP, 51.481, {lon})")  # noqa: E501

    msg, = exc.value.args
    assert msg == f'Invalid longitude: {lon}.000000'


@pytest.mark.parametrize('interval', (-1, 28801))
def test_solar_time_with_invalid_interval_provided(db, interval):
    with db.cursor() as cur:
        with pytest.raises(psycopg.errors.InternalError) as exc:
            cur.execute(f"SELECT solar_time('2023-10-06 15:30', 51.481, 7.217, {interval})")  # noqa: E501

    msg, = exc.value.args
    assert msg == f'Invalid data_interval: {interval} (allowed range: [0 - 28800])'  # noqa: E501


def test_solar_time_with_valid_interval_provided(db):
    with db.cursor() as cur:
        cur.execute("SELECT solar_time('2023-10-06 15:30', 51.481, 7.217, 3600)")  # noqa: E50
        d, = cur.fetchone()
        print(d)

    assert d == datetime(2023, 10, 6, 16, 10, 43, 425000)
