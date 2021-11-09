.load build/libngram.so
CREATE VIRTUAL TABLE t1 USING fts5(x, tokenize = 'ngram gram 2');
INSERT INTO t1 VALUES(' 2021 年 10 月，在 Ubuntu Linux 上如何使用WeChat ？ 🤣🎃');

SELECT ngram_highlight(t1, 0, '[', ']') FROM t1('ubuntu linux上');

