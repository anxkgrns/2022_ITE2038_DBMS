SELECT P1.id, P1.name, P2.name, P3.name
FROM Pokemon P1, Pokemon P2, Pokemon P3 ,Evolution E, Evolution E2
WHERE P1.id = E.before_id
AND E.after_id = E2.before_id
AND P2.id = E. after_id
AND P3.id = E2.after_id
ORDER BY P1.id ASC;

