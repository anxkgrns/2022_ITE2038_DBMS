SELECT Pokemon.name 
FROM Pokemon
WHERE Pokemon.id NOT IN 
(
  SELECT Evolution.before_id 
  FROM Evolution
) 
AND Pokemon.type IN ('Water')
ORDER BY Pokemon.name ASC;
