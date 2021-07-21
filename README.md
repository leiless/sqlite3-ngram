# HOWTO build FTS5 N-gram tokenizer

```bash
$ cmake .
$ make
# Check the libfts5_ngram_tokenizer.so
```



# HOWTO use in `td/example/cpp`

```bash
$ cd td/example/cpp/build
# Symlink libfts5_ngram_tokenizer.so to td/example/cpp/build
$ export TD_FTS_EXT=./libfts5_ngram_tokenizer
$ ./td_example
```

