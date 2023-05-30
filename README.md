# shop4AtmosData

This repo is very much WIP and follows the segfault-driven programming paradigm.
It's currently unclear, if this should develop into a library (and thus be basically a
reimplementation of the official [CDSAPI-package](https://github.com/ecmwf/cdsapi) in C) or stay in this more-like-a-standalone-program state.

Compared to the official implementation, `shop4AtmosData` is fairly limited in its ability as it's original intent was
to be used for [FORCE](https://github.com/davidfrantz/force) to serve as a replacement for MODIS-data download.

## Installation

To install the software, first install the dependencies listed below and **afterward** run `make cams-download`
inside the cloned repo. This will create an executable in your current working directory.

### Dependencies

- `cmake` and `make`
- [cURL](https://curl.se/): Install libcurl via your systems package manager
- [Jansson](https://github.com/akheron/jansson): Please follow the instructions detailed [here](https://jansson.readthedocs.io/en/latest/gettingstarted.html)
for installing `jansson`.
- [ecCodes](https://github.com/ecmwf/eccodes): Please follow the instructions detailed in the Repo's Readme to build the library either from source or a release bundle (recommended!). See [here](https://confluence.ecmwf.int/display/ECC/ecCodes+installation) for compiliation options. Please use the following flags: `-DENABLE_PNG=ON -DENABLE_ECCODES_OMP_THREADS=ON -DENABLE_MEMFS=ON`. **Current version used:** `2.30.0`.

## ADS Access

To query the ADS-API, you need to create [an account](https://ads.atmosphere.copernicus.eu/user/login?destination=/),
accept their terms of service and create a local file containing your credentials in the following format,
which is congruent with the requirements for the official [CDSAPI-package](https://github.com/ecmwf/cdsapi):

```text
url: https://ads.atmosphere.copernicus.eu/api/v2
key: <UID>:<SECRET>
verify: 1
```

The program expects an environment variable named `ADSAUTH` which points to the file created above.
Alternatively, the path to such a file can be specified at runtime via the argument `-a` or `--authentication`.

## Usage

```shell
Usage: cams-download [-h|--help] [-v|--version] [-i|--purpose] [-o|--output_directory] <-c|--coordinates> <--start> <--end> <-t|--daily_tables> <-s|--climatology> <-a|--authentication>

[-h|--help]             print this help page and exit
[-v|--version]          print version
[-i|--purpose]          show program's purpose

[-o|--output_directory] ...
Optional arguments:
<-c|--coordinates>      Path to file with WRS2 center coordinates, if subset of area is to be queried. Otherwise the entire model area is requested.
<--start>               Start date. Default: 2003-01-01.
<--end>                 Start date. Default: 2003-01-01.
<--product>             Product type to query. Currently, only REPROCESSED and FORECAST are implemented. Default is REPROCESSED
<--time>                Model times. Comma-separated list; valid range from 0 to 21 in steps of 3. Default: 0
<--lead-time-hour>      Leadtime. Comma-separated list; valid range from 0 to 120. Default: 0
<-t|--daily_tables>     build daily tables? Default: false
<-s|--climatology>      build climatology? Default: false

<-a|--authentication>   optional...
```

## Further Ideas

- Accept the path to a FORCE datacube to automatically determine the best product time to request; 
currently all requests are made for 00:00 
