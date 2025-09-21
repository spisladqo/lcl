# lcl
Library (and cli tool) for single- and multi-threaded bmp image convolution.

## Setting up
```
git clone https://github.com/spisladqo/lcl && cd lcl

make build
```

## Usage

The tool can be run from the command line.


```bash
./lcl --mode=<MODE> --src=<SOURCE_IMAGE> --target=<TARGET_IMAGE> --filter=<FILTER> [OPTIONS]
```

To learn more about flags, run `./lcl --help`.


## Tests

`make test` runs unit tests.
Running unit tests requires cmocka.

`make bench` runs benchmarks from tests/benchmarks.

## Benchmarks and experiments

Can be found in [Benchmarks.md](Benchmarks.md).

## License

This software is distributed under GPL-3.0 license. See [LICENSE](LICENSE) for more info.