.load build/libngram_porter.so
CREATE VIRTUAL TABLE t1 USING fts5(x, tokenize = 'ngram_porter');
INSERT INTO t1 VALUES(' 2021 年 10 月，在 Ubuntu Linux 上如何使用WeChat ？ ');

