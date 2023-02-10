# force-lut-cams

With the decommissioning of MODIS on the horizon, David posed the question on GitHub if anyone would like to work on a
suitable replacement. His proposed idea is the utilization of ECMWF CAMS data. While the ultimate goal should be to 
write modules which can be integrated into the FORCE repository, it may be easier for me to first develop them as 
stand-alone programs. To ease development, the different steps of 1) downloading 2) further processing are to be 
implemented in different programs. Their requirements are detailed below.

## cams-download

- follow FORCE CLI conventions, i.e. use getopts and use the following table below
- read the authentication either bei passing in the path of the corresponding file or querying a environment variable;
  - if both fail/are not present, fail with an error

| argument | description |
|:---------|:------------|
| `-i`     |             |