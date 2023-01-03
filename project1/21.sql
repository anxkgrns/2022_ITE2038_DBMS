SELECT MAX(A.cnt) max, ROUND((MAX(A.cnt)/SUM(cnt))*100,2) percent
FROM
(
  SELECT count(*) cnt
  FROM Pokemon
  GROUP BY Pokemon.type
) as A
ORDER BY max;
