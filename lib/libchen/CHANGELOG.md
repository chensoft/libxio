# Change Log
All notable changes to this project will be documented in this file(see http://keepachangelog.com/).

## [Unreleased]
### Added
- cmd: store app start info.
- proc: run a command and get its output.
- threadpool: allow fast exit when pool destroyed.

### Changed
- date: change the class interface, store current date only.
- log: use iso date format for logging.
- cmd: make the method prototype equal to main function.
- daemon: avoid the standard file descriptors to be reused.

### Fixed
- fs: resize string if user provide a large length to read.
- threadpool: all jobs should be done before exit.

## [1.1.1] - 2016-11-05
### Added
- fs: expand '~' to home directory when use realpath and absolute.
- num: check bits count of a integer.
- map: write value to variable only when it exist.
- ini: user can retrieve error position when syntax error.
- json: user can retrieve error position when syntax error.
- cmake: group files in the IDE automatically.

### Changed
- fs: accept two types of lambda callbacks.
- fs: return std::error_code in some methods.
- sys: error method return std::error_code.
- sys: error method accept your custom code.
- sys: use generic_category to keep consistency between clang and gcc.
- num: using template to implement swap for any integer.
- plat: move platform dependent codes into its related folder.

### Fixed
- sys: use XSI-compliant strerror_r only if some macro defined.

## [1.1.0] - 2016-07-22
### Added
- str: lowercase, uppercase, capitalize a string.
- str: case transform, trim methods accept const string.
- json: allow unsigned integer conversion.
- json: allow more integer type conversion.
- json: allow implicit conversion from json to variable.
- map: find value in map, a default value can be provided.
- regex: add methods support pass std::regex object.
- regex: add expand method that can use submatch to expand a string.
- iterator: a type erasure to hold any iterator.
- threadpool: user can retrieve the threads count.
- fs: user can provide cwd in absolute method.
- fs: add append method to append data to file.
- fs: remove suffix from basename if user provided.
- sys: retrieve current executable path.
- sys: retrieve current process id.
- sys: kill a specific process.
- proc: check existence of process.

### Changed
- num: using %g format float numbers.
- sys: move process related methods to proc class.
- cmd: don't set option value to "yes" if no value provided.

### Fixed
- ini: fix parse error if comment between two properties.
- str: fix replace string didn't consider the all param.
- str: fix split algorithm didn't consider no delimiter in string.
- fs: fix method absolute didn't remove ".." if path is "../a/b".

### Removed
- cmd: remove int64 and double conversion at this moment.

## [1.0.1] - 2016-06-21
### Added
- test: add threadpool unit test.
- num: check whether an integer is a prime number.

### Changed
- test: put googletest into lib folder.

### Fixed
- mt: remove clone method for callable, it will cause compilation fails.

## [1.0.0] - 2016-06-20
### Added
- ini: report error position if found syntax error.
- data: add unit test for json and ini parser.

### Changed
- num: improve double to string performance.

## [0.1.0-rc.1] - 2016-06-19
### Added
- json: add methods to modify json's data.
- json: add validate method to check if json string is valid.
- json: construct default value according to the json type.

### Fixed
- json: fix didn't use rvalue in constructors.

## [0.1.0-rc] - 2016-06-16
### Added
- utf8: convert from unicode point to utf-8 string.
- num: number to string, some platforms have no std::to_string support, so add these methods.
- fs: read and write text files.
- fs: support Windows.
- sys: thread-safe errno to string.
- cmd: check option is being set.
- log: custom logger can promote to be the current logger.

### Changed
- string: format using variadic templates.
- string: replace, remove and trim return the changed string.
- map: support both std::map and std::unordered_map.

### Fixed
- utf8: check bom before calculate character count and do sub string.

## [0.1.0-alpha] - 2016-05-30
### Added
- string: format, print, equality test, characters count, prefix & suffix check, split, join, replace, remove, trim, levenshtein algorithm.
- utf8: check utf-8 string character count, retrieve sub string, check bom.
- num: float equality test, integer swap, random number generator.
- regex: simple wrapper for regex match, replace and group.
- map: check key existence, retrieve all keys, all values, flip map's key and value.
- vector: make unique, combine, intersect and difference.
- any: a wrapper to hold any type.
- json: json file and data parser.
- ini: ini file and data parser.
- semaphore: C++11 semaphore support.
- threadpool: threadpool using std::future.
- fs: check root, home, temp, current path, change working directory.
- fs: realpath, absolute path, normalize path.
- fs: dirname, basename, extname, check file existence, check permissions.
- fs: file access time, modification time, create time.
- fs: touch file, create, rename, remove and copy file or folder.
- fs: visit folder, check folder's item count.
- sys: create daemon use a single function.
- sys: uuid generator.
- time: simple timing class.
- date: thread-safe gmt & local time, date formatter.
- log: simple logging system.
- cmd: modern command line parser.
