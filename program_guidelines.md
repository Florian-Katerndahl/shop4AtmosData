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

### Argument Parsing

### Miscellaneous

- provide a `--verbose` flag

### Authentication

- try to load the API authentication from the environment variable `ADSAUTH`
- alternatively, allow the user to provide the file path to an existing secret file
  - use the flags `-a` or `--authentication`
  - test for 1) file exists, 2) is readable, 3) belongs to the same user?
- If neither of the above-mentioned works, fail before any further parsing of queries etc. is done
  - note however, that this does not guarantee valid API credentials; at this point the only goal is to populate a
  struct with 1) the API base URL, 2) User ID, 3) API secret and 4) whether to validate SSL certs (reminiscent from the
  Python API **which I would ignore and never establish an untrusted HTTP(S)-connection**)

| argument | description |
|:---------|:------------|
| `-i`     |             |