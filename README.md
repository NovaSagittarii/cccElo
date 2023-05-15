# CCC RATING CALCULATIONS
---
dedicated to andy who does not get up at 7am to do codeforces
and so i get a bunch of data, combine them, do some math on it, to get unofficial ratings

Rating calculations are based on the Atcoder Rating System.

| Contest Type             | Rated Bound | Notes
| ------------------------ | ----------- | ---- |
| Codeforces Div 1         | 9999 | Similar to AGC            |
| Codeforces Div 2         | 2800 | Similar to ARC, cf-2100 is around ac-1800 (28/18 ~ 1.5) |
| Codeforces Div 3         | 1800 | cf-1600 is around ac-1200 (1200 x 1.5 = 1800) |
| Codeforces Div 4         | 1350 | cf-1200 is around ac-900 (900 x 1.5 = 1350) |
| Atcoder Grand Contest    | 9999 | |
| Atcoder Regular Contest  | 2800 | |
| Atcoder Beginner Contest | 2000 | |

### Folders
- `data` - raw data, json from codeforces and atcoder API
- `ndata` - lookup data (atcoder)
- `fdata` - formatted data (takes in `data` and `ndata` and dumps it here, to be processed by some c++ probably?)
- `extra` - extra data (formatted), "unofficial standings" that are mixed with `fdata` to get results

### Building
```sh
make && ./calculator
# g++ -c -fPIC calculator-lib.cpp -o foo.o
# g++ -shared -Wl,-soname,libfoo.so -o libfoo.so  foo.o
```