Rebol [
	Title:   "Rebol3 map! test script"
	Author:  "Oldes, Peter W A Wood"
	File: 	 %map-test.r3
	Tabs:	 4
	Needs:   [%../quick-test-module.r3]
]

~~~start-file~~~ "map"

===start-group=== "map issues"
	;@@ https://github.com/rebol/rebol-issues/issues/770
	--test-- "map-issue-770"
		--assert  map? m: make map! [a: 1]
		--assert  1 = m/a
		--assert  not empty? m
		--assert  empty? clear m

	;@@ https://github.com/rebol/rebol-issues/issues/598
	--test-- "map-issue-598"
		--assert error? try [make map! [[a] 1]]

===end-group===

===start-group=== "reflection"
	m: make map! b: [a 1 b: 2 :c 3 'd 4 #e 5 /f 6 "a" 7 <b> 8 9 9 #"c" 10 a@b 11]
	--test-- "body of map"
		--assert  [a  1 b: 2 :c 3 'd 4 #e 5 /f 6 "a" 7 <b> 8 9 9 #"c" 10 a@b 11] = b
		--assert  [a: 1 b: 2 c: 3 d: 4 e: 5 f: 6 "a" 7 <b> 8 9 9 #"c" 10 a@b 11] = body-of m
		--assert  [a: 1 b: 2 c: 3 d: 4 e: 5 f: 6 "a" 7 <b> 8 9 9 #"c" 10 a@b 11] = reflect m 'body

	--test-- "values of map"
		--assert  [1 2 3 4 5 6 7 8 9 10 11] = values-of m
		--assert  [1 2 3 4 5 6 7 8 9 10 11] = reflect m 'values

	--test-- "words of map"
		--assert  [a b c d e f "a" <b> 9 #"c" a@b] = words-of m
		--assert  [a b c d e f "a" <b> 9 #"c" a@b] = reflect m 'words

===end-group===

~~~end-file~~~