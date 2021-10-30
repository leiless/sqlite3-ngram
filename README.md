# `sqlite3-ngram-porter`

## Build

```bash
./download-sqlite.sh
./build.sh
```

## Usage

```bash
$ sqlite3
sqlite> .load build/libngram_porter.so

# ... are optional arguments to the ngram_porter tokenizer
sqlite> CREATE VIRTUAL TABLE t1 USING fts5(x, tokenize = 'ngram_porter ...');
sqlite> INSERT INTO t1 VALUES('2021 年 10 月，在 Ubuntu Linux 上如何使用WeChat ？');
```

