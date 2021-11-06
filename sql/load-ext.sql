.load build/libngram.so
CREATE VIRTUAL TABLE t1 USING fts5(x, tokenize = 'ngram gram 2');
INSERT INTO t1 VALUES(' 2021 年 10 月，在 Ubuntu Linux 上如何使用WeChat ？ 🤣🎃');
--INSERT INTO t1 VALUES('你好杰克');
--INSERT INTO t1 VALUES('你好Jack');
--INSERT INTO t1 VALUES('Hello杰克');
--INSERT INTO t1 VALUES('Hello Jack');

-- The following queries all match the single row in the table
SELECT * FROM t1('Ubuntu Linux');
SELECT * FROM t1('如何');
SELECT * FROM t1('在ubuntu');
SELECT * FROM t1('在Ubuntu');
SELECT * FROM t1('2021年');
SELECT * FROM t1('使用');
SELECT * FROM t1('Linux上');
SELECT * FROM t1('Linux上如');
SELECT * FROM t1('微信');
SELECT * FROM t1('🤣🎃');

SELECT highlight(t1, 0, '[', ']') FROM t1('linux上');
SELECT highlight(t1, 0, '[', ']') FROM t1('ubuntu linux上');

