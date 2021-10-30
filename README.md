# `sqlite3-ngram-porter`

## Build

```bash
./download-sqlite.sh
./build.sh
```

## Run

```bash
$ sqlite3
sqlite> .load build/libngram_porter.so

# ... are optional arguments to the ngram_porter tokenizer
sqlite> CREATE VIRTUAL TABLE mail USING fts5(sender, title, body, tokenize = 'ngram_porter ...');
```

