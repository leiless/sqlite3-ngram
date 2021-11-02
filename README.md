# `sqlite3-ngram`

## Build

```bash
./download-sqlite.sh
./build.sh
```

## Usage

```bash
$ sqlite3
sqlite> .load build/libngram.so

# ... are optional arguments to the ngram tokenizer
sqlite> CREATE VIRTUAL TABLE t1 USING fts5(x, tokenize = 'ngram ...');
sqlite> INSERT INTO t1 VALUES('2021 年 10 月，在 Ubuntu Linux 上如何使用WeChat ？');
```

