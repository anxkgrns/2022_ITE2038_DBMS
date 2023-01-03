SELECT Pokemon.type
FROM Pokemon 
WHERE Pokemon.type IN 
(
  SELECT Pokemon.type 
  FROM Pokemon 
  WHERE Pokemon.id IN 
  (
    (SELECT after_id FROM Evolution) union (SELECT before_id FROM Evolution)
  )
  GROUP BY Pokemon.type
  HAVING count(Pokemon.type)>=3
)
GROUP BY Pokemon.type
ORDER BY Pokemon.type ASC;
