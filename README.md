[![ci](https://github.com/jkittner/solpos-postgres/actions/workflows/CI.yml/badge.svg)](https://github.com/jkittner/solpos-postgres/actions/workflows/CI.yml)
[![pre-commit.ci status](https://results.pre-commit.ci/badge/github/jkittner/solpos-postgres/main.svg)](https://results.pre-commit.ci/latest/github/jkittner/solpos-postgres/main)

# solpos-postgres

This wraps [NREL's SOLPOS 2.0](https://www.nrel.gov/grid/solar-resource/solpos.html) and makes it accessible from postgres.

The SOLPOS code was vendored from: [NREL/SolarPILOT](https://github.com/NREL/SolarPILOT/tree/21a1466398ec22db24a5a838e5133da58e347b83) which is licensed under a mixed [MIT and GPLv3license](https://github.com/jkittner/solpos/blob/master/licenses/LICENSE_SolarPILOT).

## `solar_time()`

Calculate the true solar time of a date and a location.

**Required arguments**

| **Name** | **Type**         | **Description**                                  |
| -------- | ---------------- | ------------------------------------------------ |
| `ts`     | TIMESTAMPTZ      | the timestamp to be converted to solar time      |
| `lat`    | DOUBLE PRECISION | the latitude in decimal degrees of the position  |
| `lon`    | DOUBLE PRECISION | the longitude in decimal degrees of the position |

**Optional arguments**

| **Name**        | **Type**      | **Description**                                                                                                                                                                          |
| --------------- | ------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `data_interval` | INT DEFAULT 0 | Interval of a measurement period in seconds. Forces solpos to use the time and date from the interval midpoint. The INPUT time is assumed to be the **end** of the measurement interval. |

**Sample usage**

Get the solar time for a date at a selected position:

```sql
SELECT solar_time('2023-10-06 15:30+00:00', 51.481, 7.217)
```

```console
      solar_time
-----------------------
 2023-10-06 16:10:43.7
```

Get the solar time for a date at a selected position. Each timestamp is a one-hourly interval labelled at the **end** of the interval:

```sql
SELECT solar_time('2023-10-06 15:30+00:00', 51.481, 7.217, 3600)
```

```console
       solar_time
-------------------------
 2023-10-06 16:10:43.425
```

### References

**SolarPILOT**

Wagner, M.J. (2018). "SolarPILOT Open-Source Software Project: https://github.com/NREL/SolarPILOT/tree/21a1466398ec22db24a5a838e5133da58e347b83." Accessed (27/10/2022). National Renewable Energy Laboratory, Golden, Colorado.

**Astronomical Solar Position**

Michalsky, J. 1988. The Astronomical Almanac's algorithm for approximate solar position (1950-2050). Solar Energy 40 (3), 227-235.

Michalsky, J. 1988. ERRATA: The astronomical almanac's algorithm for approximate solar position (1950-2050). Solar Energy 41 (1), 113.

**Distance from Sun to Earth**

Spencer, J. W. 1971. Fourier series representation of the position of the sun. Search 2 (5), 172. NOTE: This paper gives solar position algorithms as well, but the Michalsky/Almanac algorithm above is more accurate.

**Atmospheric Refraction Correction**

Zimmerman, John C. 1981. Sun-pointing programs and their accuracy. SAND81-0761, Experimental Systems Operation Division 4721, Sandia National Laboratories, Albuquerque, NM.

**Shadow Band Correction Factor**

Drummond, A. J. 1956. A contribution to absolute pyrheliometry. Q. J. R. Meteorol.2 Soc. 82, 481-493.

**Relative Optical Air Mass**

Kasten, F. and Young, A. 1989. Revised optical air mass tables and approximation formula. Applied Optics 28 (22), 4735-4738.

**Renormalization of KT (“PRIME”)**

Perez, R., P. Ineichen, Seals, R., & Zelenka, A. 1990. Making full use of the clearness index for parameterizing hourly insolation conditions. Solar Energy 45 (2), 111-114.

**Solar Position Relative to Earth**

Iqbal, M. 1983. An Introduction to Solar Radiation. Academic Press, NY.
