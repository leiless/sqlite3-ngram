# `sqlite3-ngram`

`ngram` is a SQLite3 FTS5 [n-gram](https://en.wikipedia.org/wiki/N-gram#Examples) tokenizer, it tokenize the input text in computational linguistics level.

For the input text `Hello 新 世界`:

- `ngram = 1`

  `Hello`, `新`, `世`, `界`

- `ngram = 2`

  `Hello`, `新世`, `世界`

- `ngram = 3`

  `Hello`, `新世界`

The tokenization is based on [UTF-8](https://en.wikipedia.org/wiki/UTF-8#Encoding) character boundary.

The ngram currently support is in range `[1, 4]`, larger ngram can be supported but it's usually unnecessary.

This tokenizer extension can be used as a fallback(generic) tokenizer for FTS purpose.

## Build

```bash
container/build.sh
```

## Usage

```sql
-- First load the ngram extension
.load build/libngram.so
-- By default N = 2, valid N is in range [1, 4]
CREATE VIRTUAL TABLE t1 USING fts5(x, tokenize = 'ngram');
CREATE VIRTUAL TABLE t1 USING fts5(x, tokenize = 'ngram gram N');

-- Or check sql/load-ext.sql for example usage
-- sqlite3 < sql/load-ext.sql
```

## Advance usage

You can integrate this tokenizer with the SQLite3 official [`porter`](https://www.sqlite.org/fts5.html#porter_tokenizer) tokenizer:

```sql
.load build/libngram.so
CREATE VIRTUAL TABLE t1 USING fts5(x, tokenize = 'porter ngram gram N');
```

In such case, if you tokenized the word `direct`. `directed`, `directing`, `direction`, `directly`, etc. can all be coalesced into `direct` and thus hit a match.

## Limitation

Currently only the UTF-8 string is supported for tokenization.

## Credits

This project was inspired from the following projects:

- [wangfenjin/simple - 支持中文（简体和繁体）和拼音的 SQLite fts5 扩展](https://github.com/wangfenjin/simple)

## TODO

* [ ] Implement `ngram_highlight()` function
* [ ] Add more test cases
* [ ] Enable build & test CI

